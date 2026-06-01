#include "main/rule_dispatcher.h"

#include <Surelog/Common/FileSystem.h>
#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>
#include <uhdm/vpi_user.h>
#include <yaml-cpp/node/convert.h>      // NOLINT(misc-include-cleaner)
#include <yaml-cpp/node/detail/impl.h>  // NOLINT(misc-include-cleaner)
#include <yaml-cpp/node/emit.h>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/node/parse.h>
#include <yaml-cpp/null.h>
#include <yaml-cpp/parser.h>

#include <filesystem>
#include <functional>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <optional>

#include "utils/design_utils.h"

namespace SL = SURELOG;

namespace {
auto GetYamlConfig(const std::filesystem::path& configFile) -> YAML::Node {
  if (!configFile.empty() && std::filesystem::exists(configFile)) {
    try {
      return YAML::LoadFile(configFile);
    } catch (const std::exception& e) {
      std::cerr << "Bad config file" << "\n";
      return YAML::Node{};
    }
  }

  const std::filesystem::path configPath = DefaultConfigFileName;
  std::filesystem::path currentDir = std::filesystem::current_path();

  while (!std::filesystem::exists(currentDir / configPath)) {
    if (currentDir.parent_path() == currentDir) {
      std::cerr << "No config file" << "\n";
      return YAML::Node{};
    }
    currentDir = currentDir.parent_path();
  }

  try {
    return YAML::LoadFile(currentDir / configPath);
  } catch (const std::exception& e) {
    std::cerr << "Bad config file" << "\n";
    return YAML::Node{};
  }
}

static constexpr std::string_view checks_key_string = "Checks";

auto BuildSetOfAllSelectedRules() -> std::unordered_set<std::string_view> {
  std::unordered_set<std::string_view> set;
  for (auto& rule : RuleInfo::allRules) {
    set.insert(rule.idName);
  }
  for (auto& rule : RuleInfo::fixableRules) {
    set.insert(rule.idName);
  }
  for (auto& rule : RuleInfo::globalRules) {
    set.insert(rule.idName);
  }
  for (auto& rule : RuleInfo::fixableGlobalRules) {
    set.insert(rule.idName);
  }
  for (auto& rule : RuleInfo::uhdmRules) {
    set.insert(rule.idName);
  }
  return set;
}

auto BuildSetOfSelectedRules(const std::filesystem::path& configFile)
    -> std::unordered_set<std::string_view> {
  std::unordered_set<std::string_view> set;
  const auto yaml = GetYamlConfig(configFile);
  const auto checks = yaml[checks_key_string];
  if (!checks.IsDefined() || !checks.IsSequence()) {
    return BuildSetOfAllSelectedRules();
  }
  for (auto& check : checks) {
    const auto name = check.as<std::string_view>();
    set.insert(name);
  }
  return set;
}

}  // namespace

static void CommitDiags(std::vector<LintDiagnostic>& diags,
                        std::string_view ruleName, AutofixContext* autofix) {
  if (autofix == nullptr) {
    return;
  }
  for (auto& d : diags) {
    d.rule_id = std::string(ruleName);

    if (autofix->collector != nullptr) {
      autofix->collector->add(d);
    }

    if (autofix->replacements != nullptr && autofix->source_mgr != nullptr) {
      for (const auto& fix : d.fixes) {
        try {
          Replacement r = fixItToReplacement(fix, *autofix->source_mgr);
          r.rule_id = d.rule_id;

          std::string conflict_msg;
          if (!autofix->replacements->add(r, &conflict_msg)) {
            std::cerr << "autofix: skipped conflicting fix [" << d.rule_id
                      << "] at " << d.filepath << ":" << d.line << ":" << d.col
                      << " — " << conflict_msg << "\n";
          }
        } catch (const std::exception& e) {
          std::cerr << "autofix: cannot convert fix [" << d.rule_id << "] at "
                    << d.filepath << ":" << d.line << " — " << e.what() << "\n";
        }
      }
    }
  }
}

void RunAllRules(const SL::FileContent* fileContent, SL::ErrorContainer* errors,
                 SL::SymbolTable* symbols,
                 const std::unordered_set<std::string_view>& ruleSet) {
  for (const auto& rule : RuleInfo::allRules) {
    if (ruleSet.find(rule.idName) == ruleSet.end() && rule.check != nullptr) {
      continue;
    }
    rule.check(fileContent, errors, symbols);
  }
}

static void RunFixableRulesOnFile(
    const SL::FileContent* fC, SL::ErrorContainer* errors,
    SL::SymbolTable* symbols,
    const std::unordered_set<std::string_view>& ruleSet,
    AutofixContext* autofix) {
  if (autofix != nullptr && autofix->source_mgr != nullptr) {
    if (!autofix->source_mgr->loadFile(fC->getFileId())) {
      std::cerr
          << "autofix: warning: cannot load file for offset computation\n";
    }
  }

  FixSourceManager empty_sm;
  FixSourceManager& sm = (autofix != nullptr && autofix->source_mgr != nullptr)
                             ? *autofix->source_mgr
                             : empty_sm;

  for (const auto& rule : RuleInfo::fixableRules) {
    if (ruleSet.find(rule.idName) == ruleSet.end() && rule.check != nullptr) {
      continue;
    }

    std::vector<LintDiagnostic> diags;
    try {
      diags = rule.check(fC, errors, symbols, sm);
    } catch (const std::exception& e) {
      std::cerr << "lint: rule [" << rule.idName
                << "] threw exception: " << e.what()
                << "\nlint: skipping rule for this file\n";
      continue;
    } catch (...) {
      std::cerr << "lint: rule [" << rule.idName
                << "] threw unknown exception; skipping\n";
      continue;
    }

    CommitDiags(diags, rule.idName, autofix);
  }
}

static void RunFixableGlobalRules(
    SL::Design* design, SL::ErrorContainer* errors, SL::SymbolTable* symbols,
    const std::unordered_set<std::string_view>& ruleSet,
    AutofixContext* autofix) {
  FixSourceManager empty_sm;
  FixSourceManager& sm = (autofix != nullptr && autofix->source_mgr != nullptr)
                             ? *autofix->source_mgr
                             : empty_sm;

  if (autofix != nullptr && autofix->source_mgr != nullptr) {
    for (auto& [name, fC] : design->getAllFileContents()) {
      if (fC != nullptr) {
        if (!sm.loadFile(fC->getFileId())) {
          std::cerr
              << "autofix: warning: cannot load file for offset computation\n";
        }
      }
    }
  }

  for (const auto& rule : RuleInfo::fixableGlobalRules) {
    if (ruleSet.find(rule.idName) == ruleSet.end() && rule.check != nullptr) {
      continue;
    }

    std::vector<LintDiagnostic> diags;
    try {
      diags = rule.check(design, errors, symbols, sm);
    } catch (const std::exception& e) {
      std::cerr << "lint: rule [" << rule.idName
                << "] threw exception: " << e.what()
                << "\nlint: skipping rule\n";
      continue;
    } catch (...) {
      std::cerr << "lint: rule [" << rule.idName
                << "] threw unknown exception; skipping\n";
      continue;
    }

    CommitDiags(diags, rule.idName, autofix);
  }
}

void RunAllRulesOnDesign(SL::Design* design, const vpiHandle& uhdmDesign,
                         SL::ErrorContainer* errors, SL::SymbolTable* symbols,
                         const std::filesystem::path& configFile,
                         AutofixContext* autofix) {
                         std::optional<std::filesystem::path> uvmDir) {
  if (design == nullptr) {
    return;
  }

  auto kRuleSet = BuildSetOfSelectedRules(configFile);

  for (auto& [name, fileContent] : design->getAllFileContents()) {
    if (fileContent == nullptr) {
      continue;
    }
    if (uvmDir.has_value()) {
      const std::filesystem::path filePath =
          SL::FileSystem::getInstance()->toPath(name);
      std::error_code ec;
      const auto rel = std::filesystem::relative(filePath, *uvmDir, ec);
      if (!ec && !rel.empty() && rel.native().front() != '.') {
        continue;
      }
    }
    RunAllRules(fileContent, errors, symbols, kRuleSet);
    RunFixableRulesOnFile(fileContent, errors, symbols, kRuleSet, autofix);
  }

  UvmFilter::SetUvmDir(uvmDir);

  for (const auto& rule : RuleInfo::globalRules) {
    if (kRuleSet.find(rule.idName) == kRuleSet.end() && rule.check != nullptr) {
      continue;
    }
    rule.check(design, errors, symbols);
  }

  RunFixableGlobalRules(design, errors, symbols, kRuleSet, autofix);

  for (const auto& rule : RuleInfo::uhdmRules) {
    if (kRuleSet.find(rule.idName) == kRuleSet.end() && rule.check != nullptr) {
      continue;
    }
    rule.check(uhdmDesign, errors, symbols);
  }

  UvmFilter::ClearUvmDir();
}

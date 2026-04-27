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

#include "fix/replacement.h"
#include "fix/source_manager.h"
#include "main/lint_diagnostics.h"

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
  for (auto& rule : RuleInfo::globalRules) {
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

static void RunFixableRulesOnFile(const SL::FileContent* fC,
                                  SL::ErrorContainer* errors,
                                  SL::SymbolTable* symbols,
                                  const std::vector<FixableRule>& rules,
                                  AutofixContext* autofix) {
  if (fC == nullptr) {
    return;
  }

  if (autofix != nullptr && autofix->source_mgr != nullptr) {
    if (!autofix->source_mgr->loadFile(fC->getFileId())) {
      std::cerr
          << "autofix: warning: cannot load file for offset computation\n";
    }
  }

  for (const auto& rule : rules) {
    if (!rule.enabled) {
      continue;
    }

    FixSourceManager empty_sm;
    FixSourceManager& sm =
        (autofix != nullptr && autofix->source_mgr != nullptr)
            ? *autofix->source_mgr
            : empty_sm;

    std::vector<LintDiagnostic> diags;
    try {
      diags = rule.check(fC, errors, symbols, sm);
    } catch (const std::exception& e) {
      std::cerr << "lint: rule [" << rule.name
                << "] threw exception: " << e.what()
                << "\nlint: skipping rule for this file\n";
      continue;
    } catch (...) {
      std::cerr << "lint: rule [" << rule.name
                << "] threw unknown exception; skipping\n";
      continue;
    }

    if (autofix == nullptr) {
      continue;
    }

    for (auto& d : diags) {
      d.rule_id = std::string(rule.name);

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
                        << "] at " << d.filepath << ":" << d.line << ":"
                        << d.col << " — " << conflict_msg << "\n";
            }
          } catch (const std::exception& e) {
            std::cerr << "autofix: cannot convert fix [" << d.rule_id << "] at "
                      << d.filepath << ":" << d.line << " — " << e.what()
                      << "\n";
          }
        }
      }
    }
  }
}
  }
  for (auto& rule : fixableRules) {
    std::cout << rule.name << ": true\n";
  }
}

void RunAllRulesOnDesign(SL::Design* design, const vpiHandle& uhdmDesign,
                         SL::ErrorContainer* errors, SL::SymbolTable* symbols,
                         const std::filesystem::path& configFile, AutofixContext* autofix ) {
  if (design == nullptr) {
    return;
  }

  auto kRuleSet = BuildSetOfSelectedRules(configFile);

  for (auto& [name, fileContent] : design->getAllFileContents()) {
    if (fileContent == nullptr) {
      continue;
    }

    RunAllRules(fileContent, errors, symbols, kAllRules);
    RunFixableRulesOnFile(fileContent, errors, symbols, kFixableRules, autofix);
  }

  for (const auto& rule : RuleInfo::globalRules) {
    if (kRuleSet.find(rule.idName) == kRuleSet.end() && rule.check != nullptr) {
      continue;
    }
    rule.check(design, errors, symbols);
  }

  for (const auto& rule : RuleInfo::uhdmRules) {
    if (kRuleSet.find(rule.idName) == kRuleSet.end() && rule.check != nullptr) {
      continue;
    }
    rule.check(uhdmDesign, errors, symbols);
  }
}

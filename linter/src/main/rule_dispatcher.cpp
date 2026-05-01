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

void RunAllRulesOnDesign(SL::Design* design, const vpiHandle& uhdmDesign,
                         SL::ErrorContainer* errors, SL::SymbolTable* symbols,
                         const std::filesystem::path& configFile) {
  if (design == nullptr) {
    return;
  }

  auto kRuleSet = BuildSetOfSelectedRules(configFile);

  for (auto& [name, fileContent] : design->getAllFileContents()) {
    if (fileContent == nullptr) {
      continue;
    }
    RunAllRules(fileContent, errors, symbols, kRuleSet);
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

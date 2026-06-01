#include "utils/design_utils.h"

#include <Surelog/Common/FileSystem.h>
#include <Surelog/Common/NodeId.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <cstddef>
#include <filesystem>
#include <functional>
#include <optional>
#include <string_view>
#include <vector>

#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"

namespace SL = SURELOG;
namespace fs = std::filesystem;

namespace {
auto UvmDirStorage() -> std::optional<fs::path>& {
  static std::optional<fs::path> s_dir;
  return s_dir;
}
}  // namespace

namespace UvmFilter {

void SetUvmDir(const std::optional<fs::path>& dir) { UvmDirStorage() = dir; }
void ClearUvmDir() { UvmDirStorage() = std::nullopt; }

auto IsUvmFile(std::string_view pathStr) -> bool {
  const auto& uvmDir = UvmDirStorage();
  if (!uvmDir.has_value() || pathStr.empty()) {
    return false;
  }
  std::error_code ec;
  const auto rel = fs::relative(fs::path{pathStr}, *uvmDir, ec);
  return !ec && !rel.empty() && rel.native().front() != '.';
}

}  // namespace UvmFilter

namespace DesignUtils {

void ForEachFileContent(
    SL::Design* design,
    const std::function<void(const SL::FileContent*)>& func) {
  if (design == nullptr) {
    return;
  }
  for (auto& [name, fileContent] : design->getAllFileContents()) {
    if (fileContent == nullptr) {
      continue;
    }
    func(fileContent);
  }
}

auto ExtractDesignInfo(const SL::FileContent* fileContent,
                       SL::NodeId configDecl) -> DesignInfo {
  DesignInfo info;

  const SL::NodeId designStmt =
      fileContent->sl_get(configDecl, SL::VObjectType::paDesign_statement);
  if (!designStmt) {
    return info;
  }

  std::vector<std::string_view> identifiers;
  SL::NodeId current = fileContent->Child(designStmt);
  while (current) {
    if (fileContent->Type(current) == SL::VObjectType::slStringConst) {
      identifiers.push_back(fileContent->SymName(current));
    }
    current = fileContent->Sibling(current);
  }

  if (identifiers.empty()) {
    return info;
  }

  if (identifiers.size() == 1) {
    info.moduleName = identifiers.at(0);
    info.libName = "";
    info.scopeName = info.moduleName;
  } else {
    info.libName = identifiers.at(0);
    info.moduleName = identifiers.back();
    info.scopeName = identifiers.at(1);
    for (size_t i = 2; i < identifiers.size(); ++i) {
      info.scopeName = info.scopeName + "::" + std::string(identifiers.at(i));
    }
  }

  return info;
}

}  // namespace DesignUtils

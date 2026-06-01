// utils/design_utils.h
#pragma once

#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>

#include <filesystem>
#include <functional>
#include <optional>
#include <string_view>

struct DesignInfo {
  std::string libName;
  std::string_view moduleName;
  std::string scopeName;
};

namespace UvmFilter {

void SetUvmDir(const std::optional<std::filesystem::path>& dir);
void ClearUvmDir();
auto IsUvmFile(std::string_view pathStr) -> bool;

}  // namespace UvmFilter

namespace DesignUtils {

void ForEachFileContent(
    SURELOG::Design* design,
    const std::function<void(const SURELOG::FileContent*)>& func);

auto ExtractDesignInfo(const SURELOG::FileContent* fileContent,
                       SURELOG::NodeId configDecl) -> DesignInfo;

}  // namespace DesignUtils

// utils/design_utils.h
#pragma once

#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>

#include <functional>

struct DesignInfo {
  std::string libName;
  std::string_view moduleName;
  std::string scopeName;
};

namespace DesignUtils {

void ForEachFileContent(
    SURELOG::Design* design,
    const std::function<void(const SURELOG::FileContent*)>& func);

auto ExtractDesignInfo(const SURELOG::FileContent* fileContent,
                       SURELOG::NodeId configDecl) -> DesignInfo;

}  // namespace DesignUtils

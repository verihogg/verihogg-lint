// utils/design_utils.h
#pragma once

#include <Surelog/Design/Design.h>
#include <Surelog/Design/FileContent.h>

#include <functional>

namespace DesignUtils {

void ForEachFileContent(
    SURELOG::Design* design,
    const std::function<void(const SURELOG::FileContent*)>& fn);

}  // namespace DesignUtils
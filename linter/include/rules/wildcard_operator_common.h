#pragma once

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>

#include <cstdint>
#include <string_view>

enum class WildcardOperatorKind : std::uint8_t {
  kUnknown = 0,
  kEquality,
  kInequality,
};

auto GetWildcardLhsName(const SURELOG::FileContent* fileContent,
                        SURELOG::NodeId opNode) -> std::string_view;

auto DetectWildcardOperatorKind(const SURELOG::FileContent* fileContent,
                                SURELOG::NodeId opNode) -> WildcardOperatorKind;

#include "rules/always_event_control.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <vector>

#include "main/lint_rules.h"
#include "utils/location_utils.h"

namespace {

namespace SL = SURELOG;

bool IsAlwaysKeyword(SL::VObjectType t) {
  return t == SL::VObjectType::paALWAYS ||
         t == SL::VObjectType::paALWAYS_COMB ||
         t == SL::VObjectType::paALWAYS_FF ||
         t == SL::VObjectType::paALWAYS_LATCH;
}

bool IsPlainAlwaysKeyword(SL::VObjectType t) {
  return t == SL::VObjectType::paALWAYS;
}

SL::NodeId GetKeywordNode(const SL::FileContent* fC,
                          SL::NodeId alwaysConstruct) {
  for (SL::NodeId c = fC->Child(alwaysConstruct); c != SL::InvalidNodeId;
       c = fC->Sibling(c)) {
    if (IsAlwaysKeyword(fC->Type(c))) {
      return c;
    }
  }
  return SL::InvalidNodeId;
}

bool IsPlainAlways(const SL::FileContent* fC, SL::NodeId alwaysConstruct) {
  const SL::NodeId kw = GetKeywordNode(fC, alwaysConstruct);
  return kw != SL::InvalidNodeId && IsPlainAlwaysKeyword(fC->Type(kw));
}

SL::NodeId StatementAfterKeyword(const SL::FileContent* fC,
                                 SL::NodeId alwaysConstruct) {
  const SL::NodeId kw = GetKeywordNode(fC, alwaysConstruct);
  if (kw == SL::InvalidNodeId) {
    return SL::InvalidNodeId;
  }
  return fC->Sibling(kw);
}

SL::NodeId UnwrapStatement(const SL::FileContent* fC, SL::NodeId node) {
  while (node != SL::InvalidNodeId) {
    const SL::VObjectType t = fC->Type(node);
    if (t == SL::VObjectType::paStatement ||
        t == SL::VObjectType::paStatement_item ||
        t == SL::VObjectType::paStatement_or_null) {
      const SL::NodeId c = fC->Child(node);
      if (c == SL::InvalidNodeId) {
        return SL::InvalidNodeId;
      }
      node = c;
      continue;
    }
    return node;
  }
  return SL::InvalidNodeId;
}

}  // namespace

void CheckAlwaysEventControl(const SURELOG::FileContent* fileContent,
                             SURELOG::ErrorContainer* errors,
                             SURELOG::SymbolTable* symbols) {
  if (fileContent == nullptr) {
    return;
  }

  const std::vector<SL::NodeId> kAlwaysConstructs = fileContent->sl_collect_all(
      fileContent->getRootNode(), SL::VObjectType::paAlways_construct);

  for (const auto alwaysConstruct : kAlwaysConstructs) {
    if (!IsPlainAlways(fileContent, alwaysConstruct)) {
      continue;
    }

    const SL::NodeId statement =
        StatementAfterKeyword(fileContent, alwaysConstruct);
    const SL::NodeId unwrapped = UnwrapStatement(fileContent, statement);
    const bool ok = unwrapped != SL::InvalidNodeId &&
                    fileContent->Type(unwrapped) ==
                        SL::VObjectType::paProcedural_timing_control_statement;

    if (!ok) {
      ReportError(fileContent, alwaysConstruct, "always",
                  verihogg_lint::LINT_ALWAYS_EVENT_CONTROL, errors, symbols);
    }
  }
}

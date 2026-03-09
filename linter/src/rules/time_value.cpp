#include "rules/time_value.h"

#include <string>
#include <string_view>

#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "utils/location_utils.h"

using namespace SURELOG;

static void CheckTimeLiteral(const FileContent* fC, NodeId timeLiteral,
                             ErrorContainer* errors, SymbolTable* symbols) {
  NodeId intConst = fC->Child(timeLiteral);
  if (!intConst) return;
  if (fC->Type(intConst) != VObjectType::slIntConst) return;

  NodeId timeUnit = fC->Sibling(intConst);
  if (!timeUnit) return;
  if (fC->Type(timeUnit) != VObjectType::paTime_unit) return;

  const auto kEndOfNumber = fC->EndColumn(intConst);
  const auto kStartOfUnit = fC->Column(timeUnit);

  if (kStartOfUnit <= kEndOfNumber) return;

  const auto kNumber = fC->SymName(intConst);
  const auto kUnit = fC->SymName(timeUnit);

  std::string badValue;
  badValue.reserve(kNumber.size() + 1 + kUnit.size());
  badValue.append(kNumber).append(1, ' ').append(kUnit);

  ReportError(fC, intConst, badValue, ErrorDefinition::LINT_TIME_VALUE, errors,
              symbols);
}

void CheckTimeValue(const FileContent* fC, ErrorContainer* errors,
                    SymbolTable* symbols) {
  if (!fC || !errors || !symbols) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  for (NodeId timeLiteral :
       fC->sl_collect_all(root, VObjectType::paTime_literal)) {
    CheckTimeLiteral(fC, timeLiteral, errors, symbols);
  }
}
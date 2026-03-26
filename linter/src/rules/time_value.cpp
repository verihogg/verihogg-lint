#include "rules/time_value.h"

#include <Surelog/Common/NodeId.h>
#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string>
#include <string_view>

#include "main/lint_rules.h"
#include "utils/location_utils.h"

namespace SL = SURELOG;

namespace {
void CheckTimeLiteral(const SL::FileContent* fileContent,
                      SL::NodeId timeLiteral, SL::ErrorContainer* errors,
                      SL::SymbolTable* symbols) {
  SL::NodeId const kIntConst = fileContent->Child(timeLiteral);
  if (!kIntConst) {
    return;
  }
  if (fileContent->Type(kIntConst) != SL::VObjectType::slIntConst) {
    return;
  }

  SL::NodeId const kTimeUnit = fileContent->Sibling(kIntConst);
  if (!kTimeUnit) {
    return;
  }
  if (fileContent->Type(kTimeUnit) != SL::VObjectType::paTime_unit) {
    return;
  }

  const auto kEndOfNumber = fileContent->EndColumn(kIntConst);
  const auto kStartOfUnit = fileContent->Column(kTimeUnit);

  if (kStartOfUnit <= kEndOfNumber) {
    return;
  }

  const auto kNumber = fileContent->SymName(kIntConst);
  const auto kUnit = fileContent->SymName(kTimeUnit);

  std::string badValue;
  badValue.reserve(kNumber.size() + 1 + kUnit.size());
  badValue.append(kNumber).append(1, ' ').append(kUnit);

  ReportError(fileContent, kIntConst, badValue, verihogg_lint::LINT_TIME_VALUE,
              errors, symbols);
}
}  // namespace

void CheckTimeValue(const SL::FileContent* fileContent,
                    SL::ErrorContainer* errors, SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId const kRoot = fileContent->getRootNode();
  if (!kRoot) {
    return;
  }

  for (SL::NodeId const kTimeLiteral :
       fileContent->sl_collect_all(kRoot, SL::VObjectType::paTime_literal)) {
    CheckTimeLiteral(fileContent, kTimeLiteral, errors, symbols);
  }
}
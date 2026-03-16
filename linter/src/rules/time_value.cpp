#include "rules/time_value.h"

#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <Surelog/SourceCompile/VObjectTypes.h>

#include <string>
#include <string_view>

#include "utils/location_utils.h"

namespace SL = SURELOG;

static void CheckTimeLiteral(const SL::FileContent* fileContent,
                             SL::NodeId timeLiteral, SL::ErrorContainer* errors,
                             SL::SymbolTable* symbols) {
  SL::NodeId intConst = fileContent->Child(timeLiteral);
  if (!intConst) {
    return;
  }
  if (fileContent->Type(intConst) != SL::VObjectType::slIntConst) {
    return;
  }

  SL::NodeId timeUnit = fileContent->Sibling(intConst);
  if (!timeUnit) {
    return;
  }
  if (fileContent->Type(timeUnit) != SL::VObjectType::paTime_unit) {
    return;
  }

  const auto kEndOfNumber = fileContent->EndColumn(intConst);
  const auto kStartOfUnit = fileContent->Column(timeUnit);

  if (kStartOfUnit <= kEndOfNumber) {
    return;
  }

  const auto kNumber = fileContent->SymName(intConst);
  const auto kUnit = fileContent->SymName(timeUnit);

  std::string badValue;
  badValue.reserve(kNumber.size() + 1 + kUnit.size());
  badValue.append(kNumber).append(1, ' ').append(kUnit);

  ReportError(fileContent, intConst, badValue,
              SL::ErrorDefinition::LINT_TIME_VALUE, errors, symbols);
}

void CheckTimeValue(const SL::FileContent* fileContent,
                    SL::ErrorContainer* errors, SL::SymbolTable* symbols) {
  if (fileContent == nullptr || errors == nullptr || symbols == nullptr) {
    return;
  }

  SL::NodeId root = fileContent->getRootNode();
  if (!root) {
    return;
  }

  for (SL::NodeId timeLiteral :
       fileContent->sl_collect_all(root, SL::VObjectType::paTime_literal)) {
    CheckTimeLiteral(fileContent, timeLiteral, errors, symbols);
  }
}
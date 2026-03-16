#include "rules/fatal_rule.h"

#include <uhdm/VpiListener.h>
#include <uhdm/uhdm.h>
#include <uhdm/vpi_user.h>

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "Surelog/API/Surelog.h"
#include "Surelog/Common/FileSystem.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"

namespace SL = SURELOG;

static auto ParseVpiValue(const std::string& raw) -> std::optional<int> {
  size_t pos = raw.find(':');
  std::string toParse = (pos != std::string::npos) ? raw.substr(pos + 1) : raw;
  try {
    return std::stoi(toParse);
  } catch (...) {
    return std::nullopt;
  }
}

static auto IsIntegerConstType(int ctype) -> bool {
  static constexpr std::array kIntegerConstTypes = {
      vpiIntConst, vpiDecConst,    vpiHexConst,
      vpiOctConst, vpiBinaryConst, vpiUIntConst,
  };
  return std::ranges::any_of(kIntegerConstTypes,
                             [ctype](int type) { return type == ctype; });
}

static auto TryParseConstant(const UHDM::constant* cnst) -> std::optional<int> {
  if (!IsIntegerConstType(cnst->VpiConstType())) {
    return std::nullopt;
  }
  return ParseVpiValue(std::string(cnst->VpiValue()));
}

static auto TryParseOperation(const UHDM::operation* oper)
    -> std::optional<int> {
  int opType = oper->VpiOpType();
  if (opType != vpiPlusOp && opType != vpiMinusOp) {
    return std::nullopt;
  }
  if (oper->Operands() == nullptr || oper->Operands()->empty()) {
    return std::nullopt;
  }

  const auto* cnst = dynamic_cast<UHDM::constant*>((*oper->Operands())[0]);
  if (cnst == nullptr) {
    return std::nullopt;
  }

  auto val = ParseVpiValue(std::string(cnst->VpiValue()));
  if (!val.has_value()) {
    return std::nullopt;
  }
  return (opType == vpiMinusOp) ? -val.value() : val.value();
}

static auto ExtractFinishNumber(UHDM::any* firstArg) -> std::optional<int> {
  if (const auto* cnst = dynamic_cast<UHDM::constant*>(firstArg)) {
    return TryParseConstant(cnst);
  }
  if (const auto* oper = dynamic_cast<UHDM::operation*>(firstArg)) {
    return TryParseOperation(oper);
  }
  return std::nullopt;
}

static void ReportFatalError(const char* file, int line, int column,
                             const std::string& msg, SL::ErrorContainer* errors,
                             SL::SymbolTable* symbols) {
  SL::SymbolId sym = symbols->registerSymbol(msg);
  SL::PathId fileId = SL::FileSystem::getInstance()->toPathId(file, symbols);
  SL::Location loc(fileId, line, column, sym);
  SL::Error err(SL::ErrorDefinition::LINT_FATAL_SYSCALL, loc);
  errors->addError(err, false);
}

void FatalListener::Listen(const vpiHandle& design) {
  if (design == nullptr) {
    return;
  }
  listenDesigns({design});
}

void FatalListener::enterSys_func_call(const UHDM::sys_func_call* object,
                                       vpiHandle handle) {
  if (object == nullptr) {
    return;
  }
  if (seen_.contains(object)) {
    return;
  }
  seen_.insert(object);

  if (object->VpiName() != "$fatal") {
    return;
  }

  const char* file = nullptr;
  int line = 0;
  int column = 0;
  if (handle != nullptr) {
    file = vpi_get_str(vpiFile, handle);
    line = vpi_get(vpiLineNo, handle);
    try {
      column = vpi_get(vpiColumnNo, handle);
    } catch (...) {
      column = 0;
    }
  }

  const UHDM::VectorOfany* args = object->Tf_call_args();
  if (args == nullptr || args->empty()) {
    ReportFatalError(file, line, column, "$fatal has no arguments", errors_,
                     symbols_);
    return;
  }

  UHDM::any* firstArg = (*args)[0];
  if (firstArg == nullptr) {
    return;
  }

  auto maybeVal = ExtractFinishNumber(firstArg);
  if (maybeVal.has_value()) {
    int val = maybeVal.value();
    if (val != 0 && val != 1 && val != 2) {
      ReportFatalError(file, line, column,
                       "$fatal first argument must be 0, 1, or 2. Got " +
                           std::to_string(val),
                       errors_, symbols_);
    }
  } else {
    ReportFatalError(file, line, column, "first argument is not constant",
                     errors_, symbols_);
  }

  if (args->size() > 1) {
    const auto* secondArg = (*args)[1];
    if (dynamic_cast<const UHDM::constant*>(secondArg) == nullptr) {
      ReportFatalError(file, line, column,
                       "$fatal message is not string constant", errors_,
                       symbols_);
    }
  } else {
    ReportFatalError(file, line, column, "$fatal missing message", errors_,
                     symbols_);
  }
}

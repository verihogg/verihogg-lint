#include "rules/fatal_rule.h"

#include <Surelog/Common/FileSystem.h>
#include <Surelog/Common/PathId.h>
#include <Surelog/Common/SymbolId.h>
#include <Surelog/ErrorReporting/Error.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/ErrorReporting/Location.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <uhdm/uhdm.h>
#include <uhdm/vpi_user.h>

#include <cstddef>
#include <functional>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "main/lint_rules.h"

namespace SL = SURELOG;

namespace {

auto ParseVpiIntValue(const std::string& raw) -> std::optional<int> {
  std::string numStr = raw;
  size_t const kPos = numStr.find(':');
  if (kPos != std::string::npos) {
    numStr = numStr.substr(kPos + 1);
  }
  try {
    return std::stoi(numStr);
  } catch (...) {
    return std::nullopt;
  }
}

auto IsIntegerConstType(int ctype) -> bool {
  return ctype == vpiIntConst || ctype == vpiDecConst || ctype == vpiHexConst ||
         ctype == vpiOctConst || ctype == vpiBinaryConst ||
         ctype == vpiUIntConst;
}

auto ResolveConstantValue(const UHDM::constant* constNode)
    -> std::optional<int> {
  if (!IsIntegerConstType(constNode->VpiConstType())) {
    return std::nullopt;
  }
  return ParseVpiIntValue(std::string(constNode->VpiValue()));
}

auto ResolveOperationValue(const UHDM::operation* opNode)
    -> std::optional<int> {
  int const kOpType = opNode->VpiOpType();
  bool const kIsSignOp = (kOpType == vpiPlusOp || kOpType == vpiMinusOp);
  const UHDM::VectorOfany* operands = opNode->Operands();
  if (!kIsSignOp || operands == nullptr || operands->empty()) {
    return std::nullopt;
  }
  const auto* constNode = dynamic_cast<const UHDM::constant*>((*operands)[0]);
  if (constNode == nullptr) {
    return std::nullopt;
  }
  auto parsed = ParseVpiIntValue(std::string(constNode->VpiValue()));
  if (!parsed.has_value()) {
    return std::nullopt;
  }
  return (kOpType == vpiMinusOp) ? -(*parsed) : *parsed;
}

auto ResolveFirstArgAsInt(const UHDM::any* firstArg) -> std::optional<int> {
  if (const auto* constNode = dynamic_cast<const UHDM::constant*>(firstArg)) {
    return ResolveConstantValue(constNode);
  }
  if (const auto* opNode = dynamic_cast<const UHDM::operation*>(firstArg)) {
    return ResolveOperationValue(opNode);
  }
  return std::nullopt;
}

using ErrorReporter = std::function<void(const std::string&)>;

void ValidateFinishNumber(const UHDM::any* firstArg,
                          const ErrorReporter& report) {
  auto result = ResolveFirstArgAsInt(firstArg);
  if (!result.has_value()) {
    report("first argument is not constant");
    return;
  }
  int const kVal = *result;
  if (kVal != 0 && kVal != 1 && kVal != 2) {
    report("$fatal first argument must be 0, 1, or 2. Got " +
           std::to_string(kVal));
  }
}

void ValidateMessage(const UHDM::VectorOfany* args,
                     const ErrorReporter& report) {
  if (args->size() <= 1) {
    report("$fatal missing message");
    return;
  }
  if (dynamic_cast<UHDM::constant*>((*args)[1]) == nullptr) {
    report("$fatal message is not string constant");
  }
}

}  // namespace

void FatalListener::Listen(const vpiHandle& design) {
  if (design == nullptr) {
    return;
  }
  listenDesigns({design});
}

void FatalListener::enterSys_func_call(const UHDM::sys_func_call* object,
                                       vpiHandle handle) {
  if (object == nullptr || seen_.contains(object)) {
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

  auto report = [&](const std::string& msg) {
    SL::SymbolId const kSym = symbols_->registerSymbol(msg);
    SL::PathId const kFileId =
        SL::FileSystem::getInstance()->toPathId(file, symbols_);
    SL::Location const kLoc(kFileId, line, column, kSym);
    SL::Error err(verihogg_lint::LINT_FATAL_SYSCALL, kLoc);
    errors_->addError(err, false);
  };

  const UHDM::VectorOfany* args = object->Tf_call_args();
  if ((args == nullptr) || args->empty()) {
    report("$fatal has no arguments");
    return;
  }

  UHDM::any* firstArg = (*args)[0];
  if (firstArg == nullptr) {
    return;
  }

  ValidateFinishNumber(firstArg, report);
  ValidateMessage(args, report);
}
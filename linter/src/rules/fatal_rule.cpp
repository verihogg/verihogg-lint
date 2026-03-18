#include "rules/fatal_rule.h"

#include <Surelog/Common/FileSystem.h>
#include <Surelog/Common/PathId.h>
#include <Surelog/Common/SymbolId.h>
#include <Surelog/ErrorReporting/Error.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/ErrorReporting/ErrorDefinition.h>
#include <Surelog/ErrorReporting/Location.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <uhdm/uhdm.h>
#include <uhdm/vpi_user.h>

#include <cstddef>
#include <set>
#include <string>
#include <vector>

namespace SL = SURELOG;

void FatalListener::Listen(const vpiHandle& design) {
  if (!design) {
    return;
  }
  listenDesigns({design});
}

void FatalListener::enterSys_func_call(const UHDM::sys_func_call* object,
                                       vpiHandle handle) {
  if (!object) {
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
  if (handle) {
    file = vpi_get_str(vpiFile, handle);
    line = vpi_get(vpiLineNo, handle);
    try {
      column = vpi_get(vpiColumnNo, handle);
    } catch (...) {
      column = 0;
    }
  }

  const UHDM::VectorOfany* args = object->Tf_call_args();
  if (!args || args->empty()) {
    SL::SymbolId const msgSym =
        symbols_->registerSymbol("$fatal has no arguments");
    SL::PathId const fileId =
        SL::FileSystem::getInstance()->toPathId(file, symbols_);
    SL::Location const loc(fileId, line, column, msgSym);
    SL::Error err(SL::ErrorDefinition::LINT_FATAL_SYSCALL, loc);
    errors_->addError(err, false);
    return;
  }

  UHDM::any* firstArg = (*args)[0];
  if (!firstArg) {
    return;
  }

  int val = 0;
  bool isInteger = false;

  if (auto c = dynamic_cast<UHDM::constant*>(firstArg)) {
    int const ctype = c->VpiConstType();
    isInteger = (ctype == vpiIntConst || ctype == vpiDecConst ||
                 ctype == vpiHexConst || ctype == vpiOctConst ||
                 ctype == vpiBinaryConst || ctype == vpiUIntConst);

    if (isInteger) {
      std::string raw = std::string(c->VpiValue());
      size_t const pos = raw.find(':');
      if (pos != std::string::npos) {
        raw = raw.substr(pos + 1);
      }
      try {
        val = std::stoi(raw);
      } catch (...) {
        isInteger = false;
      }
    }
  }

  else if (auto op = dynamic_cast<UHDM::operation*>(firstArg)) {
    int const opType = op->VpiOpType();
    if ((opType == vpiPlusOp || opType == vpiMinusOp) && op->Operands() &&
        !op->Operands()->empty()) {
      if (auto c = dynamic_cast<UHDM::constant*>((*op->Operands())[0])) {
        std::string raw = std::string(c->VpiValue());
        size_t const pos = raw.find(':');
        if (pos != std::string::npos) {
          raw = raw.substr(pos + 1);
        }
        try {
          val = std::stoi(raw);
        } catch (...) {
          isInteger = false;
        }
        if (opType == vpiMinusOp) {
          val = -val;
        }
        isInteger = true;
      }
    }
  }

  if (isInteger) {
    if (!(val == 0 || val == 1 || val == 2)) {
      SL::SymbolId const obj = symbols_->registerSymbol(
          "$fatal first argument must be 0, 1, or 2. Got " +
          std::to_string(val));
      SL::PathId const fileId =
          SL::FileSystem::getInstance()->toPathId(file, symbols_);
      SL::Location const loc(fileId, line, column, obj);
      SL::Error err(SL::ErrorDefinition::LINT_FATAL_SYSCALL, loc);
      errors_->addError(err, false);
    }
  } else {
    SL::SymbolId const obj =
        symbols_->registerSymbol("first argument is not constant");
    SL::PathId const fileId =
        SL::FileSystem::getInstance()->toPathId(file, symbols_);
    SL::Location const loc(fileId, line, column, obj);
    SL::Error err(SL::ErrorDefinition::LINT_FATAL_SYSCALL, loc);
    errors_->addError(err, false);
  }

  if (args->size() > 1) {
    auto secondArg = (*args)[1];
    if (!dynamic_cast<UHDM::constant*>(secondArg)) {
      SL::SymbolId const obj =
          symbols_->registerSymbol("$fatal message is not string constant");
      SL::PathId const fileId =
          SL::FileSystem::getInstance()->toPathId(file, symbols_);
      SL::Location const loc(fileId, line, column, obj);
      SL::Error err(SL::ErrorDefinition::LINT_FATAL_SYSCALL, loc);
      errors_->addError(err, false);
    }
  } else {
    SL::SymbolId const obj = symbols_->registerSymbol("$fatal missing message");
    SL::PathId const fileId =
        SL::FileSystem::getInstance()->toPathId(file, symbols_);
    SL::Location const loc(fileId, line, column, obj);
    SL::Error err(SL::ErrorDefinition::LINT_FATAL_SYSCALL, loc);
    errors_->addError(err, false);
  }
}
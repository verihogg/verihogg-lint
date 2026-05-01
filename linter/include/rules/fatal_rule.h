#pragma once

#include <Surelog/Design/FileContent.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <uhdm/VpiListener.h>
#include <uhdm/uhdm.h>
#include <uhdm/vpi_user.h>

#include <set>

namespace SL = SURELOG;

class FatalListener : public UHDM::VpiListener {
 public:
  FatalListener(SL::ErrorContainer* errors, SL::SymbolTable* symbols)
      : errors_(errors), symbols_(symbols) {}

  void Listen(const vpiHandle& design);

  void enterSys_func_call(const UHDM::sys_func_call* object,
                          vpiHandle handle) override;

 private:
  std::set<const UHDM::sys_func_call*> seen_;
  SL::ErrorContainer* errors_;
  SL::SymbolTable* symbols_;
};

void CheckFatalSyscall(const vpiHandle& design, SL::ErrorContainer* errors,
                       SL::SymbolTable* symbols);

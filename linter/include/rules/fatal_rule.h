#pragma once

#include <uhdm/VpiListener.h>
#include <uhdm/uhdm.h>

#include <vector>

#include "Surelog/API/Surelog.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"

using namespace SURELOG;

class FatalListener : public UHDM::VpiListener {
 public:
  FatalListener(const FileContent* fC, SURELOG::ErrorContainer* errors,
                SURELOG::SymbolTable* symbols)
      : fC_(fC), errors_(errors), symbols_(symbols) {}

  void listen(const vpiHandle& design);

  void enterSys_func_call(const UHDM::sys_func_call* object,
                          vpiHandle handle) override;

 private:
  std::set<const UHDM::sys_func_call*> seen_;
  const FileContent* fC_;
  ErrorContainer* errors_;
  SymbolTable* symbols_;
};

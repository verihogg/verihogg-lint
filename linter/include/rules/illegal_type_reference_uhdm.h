#pragma once

#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <uhdm/VpiListener.h>
#include <uhdm/uhdm.h>
#include <uhdm/vpi_user.h>

#include <set>

namespace SL = SURELOG;

class IllegalTypeReferenceListener : public UHDM::VpiListener {
 public:
  IllegalTypeReferenceListener(SL::ErrorContainer* errors,
                               SL::SymbolTable* symbols)
      : errors_(errors), symbols_(symbols) {}

  void Listen(const vpiHandle& design);

  void enterStruct_typespec(const UHDM::struct_typespec* object,
                            vpiHandle handle) override;
  void enterUnion_typespec(const UHDM::union_typespec* object,
                           vpiHandle handle) override;

 private:
  void CheckMembers(const UHDM::VectorOftypespec_member* members);

  std::set<const UHDM::BaseClass*> seen_;
  SL::ErrorContainer* errors_;
  SL::SymbolTable* symbols_;
};

void CheckIllegalTypeReferenceUhdm(const vpiHandle& design,
                                   SL::ErrorContainer* errors,
                                   SL::SymbolTable* symbols);
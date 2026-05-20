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
  void enterLogic_net(const UHDM::logic_net* object, vpiHandle handle) override;
  void enterStruct_net(const UHDM::struct_net* object,
                       vpiHandle handle) override;
  void enterEnum_net(const UHDM::enum_net* object, vpiHandle handle) override;
  void enterArray_net(const UHDM::array_net* object, vpiHandle handle) override;
  void enterPacked_array_net(const UHDM::packed_array_net* object,
                             vpiHandle handle) override;
  void enterInteger_net(const UHDM::integer_net* object,
                        vpiHandle handle) override;
  void enterTime_net(const UHDM::time_net* object, vpiHandle handle) override;

 private:
  void CheckMembers(const UHDM::VectorOftypespec_member* members);
  void CheckNet(const UHDM::nets* net);

  std::set<const UHDM::BaseClass*> seen_;
  SL::ErrorContainer* errors_;
  SL::SymbolTable* symbols_;
};

void CheckIllegalTypeReferenceUhdm(const vpiHandle& design,
                                   SL::ErrorContainer* errors,
                                   SL::SymbolTable* symbols);
#include "rules/illegal_type_reference_uhdm.h"

#include <Surelog/Common/FileSystem.h>
#include <Surelog/Common/PathId.h>
#include <Surelog/Common/SymbolId.h>
#include <Surelog/ErrorReporting/Error.h>
#include <Surelog/ErrorReporting/ErrorContainer.h>
#include <Surelog/ErrorReporting/Location.h>
#include <Surelog/SourceCompile/SymbolTable.h>
#include <uhdm/BaseClass.h>
#include <uhdm/ref_typespec.h>
#include <uhdm/struct_typespec.h>
#include <uhdm/typespec.h>
#include <uhdm/typespec_member.h>
#include <uhdm/uhdm_types.h>
#include <uhdm/union_typespec.h>

#include <algorithm>
#include <array>
#include <string>

#include "main/lint_rules.h"

namespace SL = SURELOG;

namespace {

constexpr std::array kIllegalTypespecKinds = {
    UHDM::UHDM_OBJECT_TYPE::uhdmreal_typespec,
    UHDM::UHDM_OBJECT_TYPE::uhdmshort_real_typespec,
    UHDM::UHDM_OBJECT_TYPE::uhdmstring_typespec,
    UHDM::UHDM_OBJECT_TYPE::uhdmchandle_typespec,
    UHDM::UHDM_OBJECT_TYPE::uhdmevent_typespec,
};

auto IsIllegalActual(const UHDM::typespec* actual) -> bool {
  if (actual == nullptr) {
    return false;
  }
  return std::ranges::find(kIllegalTypespecKinds, actual->UhdmType()) !=
         kIllegalTypespecKinds.end();
}

}  // namespace

void CheckIllegalTypeReferenceUhdm(const vpiHandle& design,
                                   SL::ErrorContainer* errors,
                                   SL::SymbolTable* symbols) {
  if (errors == nullptr || symbols == nullptr) {
    return;
  }
  IllegalTypeReferenceListener listener(errors, symbols);
  listener.Listen(design);
}

void IllegalTypeReferenceListener::Listen(const vpiHandle& design) {
  if (design == nullptr) {
    return;
  }
  listenDesigns({design});
}

void IllegalTypeReferenceListener::enterStruct_typespec(
    const UHDM::struct_typespec* object, vpiHandle /*handle*/) {
  if (object == nullptr || seen_.contains(object)) {
    return;
  }
  seen_.insert(object);
  if (!object->VpiPacked()) {
    return;
  }
  CheckMembers(object->Members());
}

void IllegalTypeReferenceListener::enterUnion_typespec(
    const UHDM::union_typespec* object, vpiHandle /*handle*/) {
  if (object == nullptr || seen_.contains(object)) {
    return;
  }
  seen_.insert(object);
  if (!object->VpiPacked()) {
    return;
  }
  CheckMembers(object->Members());
}

void IllegalTypeReferenceListener::CheckMembers(
    const UHDM::VectorOftypespec_member* members) {
  if (members == nullptr) {
    return;
  }
  for (const UHDM::typespec_member* member : *members) {
    if (member == nullptr) {
      continue;
    }
    const UHDM::ref_typespec* ref = member->Typespec();
    if (ref == nullptr) {
      continue;
    }
    const UHDM::typespec* actual = ref->Actual_typespec();
    if (!IsIllegalActual(actual)) {
      continue;
    }

    const std::string name(member->VpiName());
    SL::SymbolId const kSym = symbols_->registerSymbol(name);
    SL::PathId const kFileId = SL::FileSystem::getInstance()->toPathId(
        std::string(member->VpiFile()), symbols_);
    SL::Location const kLoc(kFileId, static_cast<uint32_t>(member->VpiLineNo()),
                            static_cast<uint16_t>(member->VpiColumnNo()), kSym);
    SL::Error err(
        verihogg_lint::LintId(verihogg_lint::LINT_ILLEGAL_TYPE_REFERENCE_UHDM),
        kLoc);
    errors_->addError(err, false);
  }
}
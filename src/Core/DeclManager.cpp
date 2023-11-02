#include "DeclManager.h"
#include "DeclTemplate.h"

CDeclManager& g_decl_manager = GetDeclManager();

CDeclManager& GetDeclManager() {
  static CDeclManager s_decl_manager;
  return s_decl_manager;
}

namespace GDecl {
#define STATIC_BUILTIN_TYPE_LAMBDA(BuiltinType)                                 \
  []() -> RBuiltin& {                                                           \
    static TBuiltin<BuiltinType> s_builtin_type(#BuiltinType, &g_decl_manager); \
    return s_builtin_type;                                                      \
  }

RBuiltin& Boolean = STATIC_BUILTIN_TYPE_LAMBDA(bool)();
RBuiltin& Int8 = STATIC_BUILTIN_TYPE_LAMBDA(int8)();
RBuiltin& Int16 = STATIC_BUILTIN_TYPE_LAMBDA(int16)();
RBuiltin& Int32 = STATIC_BUILTIN_TYPE_LAMBDA(int32)();
RBuiltin& Int64 = STATIC_BUILTIN_TYPE_LAMBDA(int64)();
RBuiltin& Uint8 = STATIC_BUILTIN_TYPE_LAMBDA(uint8)();
RBuiltin& Uint16 = STATIC_BUILTIN_TYPE_LAMBDA(uint16)();
RBuiltin& Uint32 = STATIC_BUILTIN_TYPE_LAMBDA(uint32)();
RBuiltin& Uint64 = STATIC_BUILTIN_TYPE_LAMBDA(uint64)();
RBuiltin& Float = STATIC_BUILTIN_TYPE_LAMBDA(float)();
RBuiltin& Double = STATIC_BUILTIN_TYPE_LAMBDA(double)();

#undef STATIC_BUILTIN_TYPE_LAMBDA

class _ {};
template <>
struct TCodeContainer<_> {
  TCodeContainer() {
    static TClass<std::string> s_string_type("string", static_cast<RNamespace*>(nullptr), &g_decl_manager);
    s_string_type.ASDecl = "string";
    StringType = &s_string_type;
  }
  RClass* StringType{nullptr};
};
TCodeContainer<_> code_generator;
RClass& String = *code_generator.StringType;

}  // namespace GDecl
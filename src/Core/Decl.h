#pragma once
#include "DeclFwd.h"

struct XCORE_API RDecl {
 public:
  FORCEINLINE RDecl(const std::string& name) : Name(name), DeclFlags(EDeclFlags::NoneFlagBits) {}
  FORCEINLINE virtual ~RDecl() {}
  FORCEINLINE auto GetOwner() -> RDecl*;
  FORCEINLINE auto SetOwner(RDecl* owner) -> void;
  FORCEINLINE auto GetName() -> const std::string&;
  FORCEINLINE auto GetFullName() -> std::string;
  FORCEINLINE auto GetMetadata(const std::string& key) -> const std::string&;
  FORCEINLINE auto SetMetadata(const std::string& key, const std::string& value) -> bool;
  FORCEINLINE auto InitMetadataMap(const std::unordered_map<std::string, std::string>& decl_map) -> void;
  FORCEINLINE auto GetFlags() -> uint64;
  FORCEINLINE auto SetFlags(uint64 new_flags) -> void;
  FORCEINLINE auto ClearFlags(uint64 new_flags) -> void;
  FORCEINLINE auto HasAnyFlags(uint64 check_flags) const -> bool;
  FORCEINLINE auto HasAllFlags(uint64 check_flags) const -> bool;
  FORCEINLINE auto GetDeclFlags() -> EDeclFlags;
  FORCEINLINE auto SetDeclFlags(EDeclFlags new_flags) -> void;
  FORCEINLINE auto ClearDeclFlags(EDeclFlags new_flags) -> void;
  FORCEINLINE auto HasAnyDeclFlags(EDeclFlags check_flags) const -> bool;
  FORCEINLINE auto HasAllDeclFlags(EDeclFlags check_flags) const -> bool;
  FORCEINLINE auto IsStatic() -> bool;

 protected:
  RDecl* Owner{nullptr};
  std::string Name;
  std::unique_ptr<std::unordered_map<std::string, std::string>> MetadataMap;
  union {
    uint64 Flags;
    EDeclFlags DeclFlags;
  };
  std::string_view ASDecl;

 private:
  friend class CScriptManager;
  friend class CDeclManager;
  template <typename T>
  friend struct TCodeContainer;
};

struct XCORE_API RNamespace : public RDecl {
 public:
  FORCEINLINE RNamespace(const std::string& name) : RDecl(name) {}
  FORCEINLINE auto AddDecl(RDecl* decl) -> void;

 protected:
  std::vector<RDecl*> Decls;
};

struct XCORE_API RType : public RDecl {
 public:
  FORCEINLINE RType(const std::string& name, uint32 size) : RDecl(name), Size(size) {}
  FORCEINLINE auto GetSize() -> uint32;

 protected:
  uint32 Size{0};
  ICppTraits CppTraits;
  asUINT ASTraitsFlag = 0;

 private:
  friend class CDeclManager;
};

struct XCORE_API RBuiltin : public RType {
 public:
  FORCEINLINE RBuiltin(const std::string& name, uint32 size) : RType(name, size) {}
};

struct XCORE_API REnum : public RType {
  struct SEnumValue {
    std::string_view Name;
    union {
      uint64 U64;
      int64 S64;
      uint32 U32;
      int32 S32;
    };
    FORCEINLINE SEnumValue(const char* name, uint64 val) : Name(name), U64(val) {}
    FORCEINLINE SEnumValue(const char* name, int64 val) : Name(name), S64(val) {}
  };

 public:
  FORCEINLINE REnum(const std::string& name, uint32 size) : RType(name, size) {}

 protected:
  std::vector<SEnumValue> EnumValues;

 private:
  template <typename T>
  friend struct TCodeContainer;
  friend class CScriptManager;
};

struct XCORE_API RStruct : public RType {
 public:
  FORCEINLINE RStruct(const std::string& name, uint32 size) : RType(name, size) {}
  FORCEINLINE auto SetBase(RStruct* base) -> void;
  FORCEINLINE auto AddProperty(RProperty* prop) -> void;
  FORCEINLINE auto GetKindId() -> uint32;
  FORCEINLINE auto IsA(RStruct* from) -> bool;

 protected:
  RStruct* BaseType;
  std::vector<RStruct*> DerivedTypes;
  struct {
    union {
      uint32 Bg;
      uint32 KindId;
    };
    uint32 Ed;
  } KindRange;
  RProperty* EndProp = nullptr;

 private:
  friend struct RProperty;
  friend class CDeclManager;
  friend class CScriptManager;
};

struct XCORE_API RClass : public RStruct {
 public:
  FORCEINLINE RClass(const std::string& name, uint32 size) : RStruct(name, size) {}
  FORCEINLINE auto AddFunction(RFunction* func) -> void;

 protected:
  RFunction* EndFunc = nullptr;

 private:
  friend struct RFunction;
  friend class CScriptManager;
};

struct XCORE_API RProperty : public RDecl {
 public:
  FORCEINLINE RProperty(const std::string& name, void* var_ptr) : RDecl(name), VariablePtr(var_ptr) {}
  FORCEINLINE RProperty(const std::string& name, uintptr_t offset) : RDecl(name), Offset(offset) {}

 protected:
  RType* PropType{nullptr};
  RProperty* PrevProp{nullptr};
  union {
    void* VariablePtr;
    uintptr_t Offset;
  };

 private:
  friend class CDeclManager;
  friend class CScriptManager;
  friend class RStruct;
  friend class RClass;
  friend class RFunction;
  template <typename T>
  friend struct TCodeContainer;
};

struct XCORE_API RFunction : public RStruct {
 public:
  FORCEINLINE RFunction(const std::string& name, asSFuncPtr func_ptr) : RStruct(name, 0) {}

  RFunction* PrevFunc{nullptr};
  asSFuncPtr FuncPtr;

 private:
  template <typename T>
  friend struct TCodeContainer;
};

// RDecl
auto RDecl::GetOwner() -> RDecl* {
  return Owner;
};

auto RDecl::SetOwner(RDecl* owner) -> void {
  X_ASSERT(Owner == nullptr);
  Owner = owner;
}

auto RDecl::GetName() -> const std::string& {
  return Name;
};

auto RDecl::GetFullName() -> std::string {
  return Owner ? fmt::format("{}::{}", Owner->GetFullName(), Name) : Name;
}

auto RDecl::GetMetadata(const std::string& key) -> const std::string& {
  if (MetadataMap) {
    auto it = MetadataMap->find(key);
    if (it != MetadataMap->end()) {
      return it->second;
    }
  }
  return GString::Empty;
}

auto RDecl::SetMetadata(const std::string& key, const std::string& value) -> bool {
  if (!MetadataMap) {
    MetadataMap = std::unique_ptr<std::unordered_map<std::string, std::string>>();
  }
  return MetadataMap->insert_or_assign(key, value).second;
}

auto RDecl::InitMetadataMap(const std::unordered_map<std::string, std::string>& decl_map = {}) -> void {
  assert(!MetadataMap);
  MetadataMap = std::make_unique<std::unordered_map<std::string, std::string>>(decl_map);
  MetadataMap->insert({{"", ""}});
}

auto RDecl::GetFlags() -> uint64 {
  return Flags;
}

auto RDecl::SetFlags(uint64 new_flags) -> void {
  Flags |= new_flags;
}

auto RDecl::ClearFlags(uint64 new_flags) -> void {
  Flags &= ~new_flags;
}
auto RDecl::HasAnyFlags(uint64 check_flags) const -> bool {
  return (Flags & check_flags) != 0;
}

auto RDecl::HasAllFlags(uint64 check_flags) const -> bool {
  return ((Flags & check_flags) == check_flags);
}

auto RDecl::GetDeclFlags() -> EDeclFlags {
  return DeclFlags;
}

auto RDecl::SetDeclFlags(EDeclFlags new_flags) -> void {
  DeclFlags |= new_flags;
}

auto RDecl::ClearDeclFlags(EDeclFlags new_flags) -> void {
  DeclFlags &= ~new_flags;
}

auto RDecl::HasAnyDeclFlags(EDeclFlags check_flags) const -> bool {
  return (DeclFlags & check_flags) != EDeclFlags::NoneFlagBits;
}

auto RDecl::HasAllDeclFlags(EDeclFlags check_flags) const -> bool {
  return ((DeclFlags & check_flags) == check_flags);
}

auto RDecl::IsStatic() -> bool {
  return HasAllDeclFlags(EDeclFlags::Static);
}

// RNamespace
auto RNamespace::AddDecl(RDecl* decl) -> void {
  decl->SetOwner(this);
  Decls.push_back(decl);
}

// RType
auto RType::GetSize() -> uint32 {
  return Size;
}

// RBuiltin

// REnum

// RStruct
auto RStruct::SetBase(RStruct* base_type) -> void {
  X_ASSERT(BaseType == nullptr);
  BaseType = base_type;
  base_type->DerivedTypes.push_back(this);
}

auto RStruct::AddProperty(RProperty* prop) -> void {
  X_ASSERT(prop->PrevProp == nullptr);
  prop->SetOwner(this);
  prop->PrevProp = EndProp;
  EndProp = prop;
}

auto RStruct::GetKindId() -> uint32 {
  return KindRange.KindId;
}
auto RStruct::IsA(RStruct* from) -> bool {
  return KindRange.Bg <= from->GetKindId() && from->GetKindId() < KindRange.Ed;
}

// RClass
FORCEINLINE auto RClass::AddFunction(RFunction* func) -> void {
  X_ASSERT(func->PrevFunc == nullptr);
  func->SetOwner(this);
  func->PrevFunc = EndFunc;
  EndFunc = func;
}

// RProperty

// RFunction
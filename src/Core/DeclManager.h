#pragma once
#include "Decl.h"
#include "DeclVector.h"
#include "DeclUnorderedMap.h"

class CDeclManager;
XCORE_API extern CDeclManager& g_decl_manager;
XCORE_API CDeclManager& GetDeclManager();

namespace GDecl {
XCORE_API extern RBuiltin& Boolean;
XCORE_API extern RBuiltin& Int8;
XCORE_API extern RBuiltin& Int16;
XCORE_API extern RBuiltin& Int32;
XCORE_API extern RBuiltin& Int64;
XCORE_API extern RBuiltin& Uint8;
XCORE_API extern RBuiltin& Uint16;
XCORE_API extern RBuiltin& Uint32;
XCORE_API extern RBuiltin& Uint64;
XCORE_API extern RBuiltin& Float;
XCORE_API extern RBuiltin& Double;
XCORE_API extern RClass& String;
}  // namespace GDecl

class XCORE_API CDeclManager {
 private:
  struct STypeData {
    std::string Name;
    RType* Type = nullptr;
    std::list<std::function<void(RType*)>> OnTypeRegisteredCallbacks;
  };
  CDeclManager() = default;
  CDeclManager(const CDeclManager& other) = delete;
  CDeclManager& operator=(const CDeclManager& other) = delete;

 public:
  FORCEINLINE ~CDeclManager() = default;
  FORCEINLINE auto Init() -> void;
  FORCEINLINE auto Exit() -> void;
  FORCEINLINE auto FindType(const std::string& name) -> RType*;
  FORCEINLINE auto FindEnum(const std::string& name) -> REnum*;
  FORCEINLINE auto FindClass(const std::string& name) -> RClass*;
  FORCEINLINE auto FindStruct(const std::string& name) -> RStruct*;
  FORCEINLINE auto FindProperty(const std::string& name) -> RProperty*;
  FORCEINLINE auto FindFunction(const std::string& name) -> RFunction*;
  FORCEINLINE auto GetDeclMap() -> const std::unordered_map<std::string, RDecl*>&;
  FORCEINLINE auto GetNamespace(const std::string& ns) -> RNamespace*;
  FORCEINLINE auto RegisterGlobalVariables(RProperty* global_var) -> void;

  template <typename T>
  FORCEINLINE auto FindDecl(const std::string& name) -> T*;
  template <typename T>
  FORCEINLINE auto DelaySetType(std::function<void(RType*)>&& set_type_callback) -> void;
  template <typename T, typename B = void>
  FORCEINLINE auto RegisterType(RType* type) -> void;
  template <typename PropType>
  FORCEINLINE auto RegisteProperty(RProperty* prop) -> void;
  template <typename FuncType>
  FORCEINLINE auto RegisterFunction(RFunction* func) -> void;
  template <typename V>
  FORCEINLINE auto CreateVectorType() -> RVectorType*;
  template <typename M>
  FORCEINLINE auto CreateUnorderedMapType() -> RUnorderedMapType*;

 protected:
  std::vector<std::unique_ptr<RType>> TemplateSpecializationTypes;
  std::vector<std::unique_ptr<RNamespace>> Namespaces;
  std::vector<RProperty*> GlobalVariables;
  std::unordered_map<std::string, RDecl*> DeclMap;
  std::unordered_map<std::type_index, STypeData> TypeDataMap;

 private:
  friend XCORE_API CDeclManager& GetDeclManager();
  friend class CScriptManager;
};

auto CDeclManager::Init() -> void {
  struct FInheritanceTreeIterator {
    void operator()(RStruct* it) {
      it->KindRange.Bg = KindIdCounter++;
      for (auto derived_type : it->DerivedTypes) {
        (*this)(derived_type);
      }
      it->KindRange.Ed = KindIdCounter;
    }
    uint32_t KindIdCounter = 0;
  } inheritance_tree_iterator;
  std::stack<RStruct*> struct_stack;
  for (auto& [type_index, type_data] : TypeDataMap) {
    RStruct* struct_it = dynamic_cast<RStruct*>(type_data.Type);
    if (struct_it && nullptr == struct_it->BaseType) {
      inheritance_tree_iterator(struct_it);
    }
  }
}
auto CDeclManager::Exit() -> void {}
auto CDeclManager::FindType(const std::string& name) -> RType* {
  return FindDecl<RType>(name);
}
auto CDeclManager::FindEnum(const std::string& name) -> REnum* {
  return FindDecl<REnum>(name);
}
auto CDeclManager::FindClass(const std::string& name) -> RClass* {
  return FindDecl<RClass>(name);
}
auto CDeclManager::FindStruct(const std::string& name) -> RStruct* {
  return FindDecl<RStruct>(name);
}
auto CDeclManager::FindProperty(const std::string& name) -> RProperty* {
  return FindDecl<RProperty>(name);
}
auto CDeclManager::FindFunction(const std::string& name) -> RFunction* {
  return FindDecl<RFunction>(name);
}

auto CDeclManager::GetDeclMap() -> const std::unordered_map<std::string, RDecl*>& {
  return DeclMap;
}

auto CDeclManager::GetNamespace(const std::string& ns) -> RNamespace* {
  auto ns_it = DeclMap.find(ns);
  if (ns_it != DeclMap.end()) {
    if (auto ns_decl = static_cast<RNamespace*>(ns_it->second)) {
      return ns_decl;
    } else {
      X_ASSERT(false, "this decl must is namespace decl");
    }
  } else {
    RNamespace* find_namespace = nullptr;
    size_t find_offset = 0;
    size_t find_pos = ns.find("::", find_offset);
    while (find_pos != std::string::npos) {
      auto ns_it = DeclMap.find(ns.substr(0, find_pos));
      if (ns_it == DeclMap.end()) {
        Namespaces.push_back(std::make_unique<RNamespace>(ns.substr(find_offset, find_pos - find_offset)));
        DeclMap.insert(std::make_pair(ns, Namespaces.back().get()));
        if (find_namespace) {
          find_namespace->AddDecl(Namespaces.back().get());
        }
        find_namespace = Namespaces.back().get();
      }
      find_offset = find_pos + 2;
      find_pos = ns.find("::", find_offset);
    }
    if (find_offset < ns.size()) {
      Namespaces.push_back(std::make_unique<RNamespace>(ns.substr(find_offset)));
      DeclMap.insert(std::make_pair(ns, Namespaces.back().get()));
      if (find_namespace) {
        find_namespace->AddDecl(Namespaces.back().get());
      }
      find_namespace = Namespaces.back().get();
    }
    return find_namespace;
  }
  return nullptr;
}

auto CDeclManager::RegisterGlobalVariables(RProperty* global_var) -> void {
  GlobalVariables.push_back(global_var);
}

template <typename T>
T* CDeclManager::FindDecl(const std::string& name) {
  auto iterator = DeclMap.find(name);
  if (iterator != DeclMap.end()) return dynamic_cast<T*>(iterator->second);
  return nullptr;
}

template <typename T>
auto CDeclManager::DelaySetType(std::function<void(RType*)>&& set_type_callback) -> void {
  auto type_index = std::type_index(typeid(T));
  auto it = TypeDataMap.find(type_index);
  if (it == TypeDataMap.end()) {
    it = TypeDataMap.insert_or_assign(type_index, STypeData()).first;
  }
  if (it->second.Type == nullptr) {
    it->second.OnTypeRegisteredCallbacks.emplace_back(std::move(set_type_callback));
  } else {
    set_type_callback(it->second.Type);
  }
}

template <typename T, typename B>
auto CDeclManager::RegisterType(RType* type) -> void {
  static_assert(std::is_same_v<B, void> || (!std::is_same_v<B, T> && std::is_base_of_v<B, T>));
  std::string full_name = type->GetFullName();
  X_ASSERT(!DeclMap.contains(full_name));
  DeclMap.insert(std::pair(full_name, type));

  auto type_index = std::type_index(typeid(T));
  auto type_it = TypeDataMap.find(type_index);
  if (type_it == TypeDataMap.end()) {
    type_it = TypeDataMap.insert_or_assign(type_index, STypeData()).first;
  }
  STypeData& type_data = type_it->second;
  type_data.Type = type;
  for (auto on_type_registered_callback : type_data.OnTypeRegisteredCallbacks) {
    on_type_registered_callback(type);
  }
  type_data.OnTypeRegisteredCallbacks.clear();

  if constexpr (!std::is_same_v<B, void>) {
    DelaySetType<B>([=](RType* Type) { static_cast<RStruct*>(type)->SetBase(static_cast<RStruct*>(Type)); });
  }
}

template <typename PropType>
auto CDeclManager::RegisteProperty(RProperty* prop) -> void {
  std::string full_name = prop->GetFullName();
  X_ASSERT(!DeclMap.contains(full_name));
  DeclMap.insert(std::pair(full_name, prop));

  auto type_index = std::type_index(typeid(PropType));
  auto it = TypeDataMap.find(type_index);
  if (it == TypeDataMap.end()) {
    it = TypeDataMap.insert_or_assign(type_index, STypeData()).first;
  }
  if (it->second.Type == nullptr) {
    if constexpr (IsStdVector<PropType>) {
      prop->PropType = it->second.Type = CreateVectorType<PropType>();
    } else if constexpr (IsStdUnorderedMap<PropType>) {
      prop->PropType = it->second.Type = CreateUnorderedMapType<PropType>();
    } else {
      it->second.OnTypeRegisteredCallbacks.push_back([=](RType* Type) { prop->PropType = Type; });
    }
  } else {
    prop->PropType = it->second.Type;
  }
}

template <typename FuncType>
auto CDeclManager::RegisterFunction(RFunction* func) -> void {
  std::string full_name = func->GetFullName();
  X_ASSERT(!DeclMap.contains(full_name));
  DeclMap.insert(std::pair(full_name, func));
}

template <typename V>
auto CDeclManager::CreateVectorType() -> RVectorType* {
  using E = typename TStdVector<V>::EleType;
  TVectorType<E>* vector_type = new TVectorType<E>();
  TemplateSpecializationTypes.emplace_back(std::unique_ptr<TVectorType<E>>(vector_type));
  DelaySetType<E>([=](RType* Type) { vector_type->EleType = Type; });
  return vector_type;
}

template <typename M>
auto CDeclManager::CreateUnorderedMapType() -> RUnorderedMapType* {
  using K = typename TStdUnorderedMap<M>::KeyType;
  using V = typename TStdUnorderedMap<M>::ValType;
  TUnorderedMapType<K, V>* unordered_map_type = new TUnorderedMapType<K, V>();
  TemplateSpecializationTypes.emplace_back(std::unique_ptr<TUnorderedMapType<K, V>>(unordered_map_type));
  DelaySetType<K>([=](RType* Type) { unordered_map_type->KeyType = Type; });
  DelaySetType<V>([=](RType* Type) { unordered_map_type->ValType = Type; });
  return &unordered_map_type;
}

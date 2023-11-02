#pragma once
#include "DeclManager.h"

template <typename T>
struct TBuiltin : public RBuiltin {
 public:
  TBuiltin(const std::string& name, CDeclManager* decl_manager) : RBuiltin(name, sizeof(T)) {
    decl_manager->template RegisterType<T>(this);
  }
};

template <typename E>
struct TEnum : public REnum {
  FORCEINLINE TEnum(const std::string& name, RNamespace* owner, CDeclManager* decl_manager) : REnum(name, sizeof(E)) {
    if (owner) {
      owner->AddDecl(this);
    }
    decl_manager->RegisterType<E>(this);
  }
  FORCEINLINE TEnum(const std::string& name, const char* owner, CDeclManager* decl_manager) : REnum(name, sizeof(E)) {
    if (owner) {
      if (RNamespace* namespace_owner = decl_manager->GetNamespace(owner)) {
        namespace_owner->AddDecl(this);
      }
    }
    decl_manager->RegisterType<E>(this);
  }
};

template <typename C, typename B = void>
struct TClass : public RClass {
  FORCEINLINE void Init(CDeclManager* decl_manager) {
    X_ASSERT(decl_manager);
    if constexpr (std::is_base_of_v<OObject, C>) {
      DeclFlags |= EDeclFlags::Object;
    }
    if constexpr (std::is_base_of_v<ORefCntObject, C>) {
      DeclFlags |= EDeclFlags::RefCntObject;
    }
    CppTraits = TCppTraits<C>();
    ASTraitsFlag = asGetTypeTraits<C>();
    decl_manager->RegisterType<C>(this);
  }
  FORCEINLINE TClass(const std::string& name, RNamespace* owner, CDeclManager* decl_manager) : RClass(name, sizeof(C)) {
    if (owner) {
      owner->AddDecl(this);
    }
    Init(decl_manager);
  }
  FORCEINLINE TClass(const std::string& name, const char* owner, CDeclManager* decl_manager) : RClass(name, sizeof(C)) {
    if (owner) {
      if (RNamespace* namespace_owner = decl_manager->GetNamespace(owner)) {
        namespace_owner->AddDecl(this);
      }
    }
    Init(decl_manager);
  }
};

template <typename T>
struct TMemberVariable : public RProperty {
  FORCEINLINE TMemberVariable(const std::string& name, uintptr_t offset, RStruct* owner, CDeclManager* decl_manager)
      : RProperty(name, offset) {
    owner->AddProperty(this);
    decl_manager->RegisteProperty<T>(this);
  }
};

template <typename T>
struct TGlobalVariable : public RProperty {
  FORCEINLINE void Init(CDeclManager* decl_manager) {
    DeclFlags |= EDeclFlags::Static;
    decl_manager->RegisterGlobalVariables(this);
    decl_manager->RegisteProperty<T>(this);
  }
  FORCEINLINE explicit TGlobalVariable(const std::string& name, void* var_ptr, RStruct* owner, CDeclManager* decl_manager)
      : RProperty(name, var_ptr) {
    X_ASSERT(owner != nullptr);
    Owner = owner;
    Init(decl_manager);
  }
  FORCEINLINE explicit TGlobalVariable(const std::string& name, void* var_ptr, RNamespace* owner, CDeclManager* decl_manager)
      : RProperty(name, var_ptr) {
    if (owner) {
      owner->AddDecl(this);
    }
    Init(decl_manager);
  }
  FORCEINLINE explicit TGlobalVariable(const std::string& name, void* var_ptr, const char* owner, CDeclManager* decl_manager)
      : RProperty(name, var_ptr) {
    if (owner) {
      if (RNamespace* namespace_owner = decl_manager->GetNamespace(owner)) {
        namespace_owner->AddDecl(this);
      }
    }
    Init(decl_manager);
  }
};

template <typename F>
struct TMemberFunction : public RFunction {
  constexpr static size_t ParametersNum = boost::function_types::function_arity<F>::value;
  using ReturnCppType = typename boost::function_types::result_type<F>::type;
  template <size_t I>
  using ParameterCppTypes = typename boost::mpl::at_c<boost::function_types::parameter_types<F>, I>::type;

  template <size_t I>
  void MakeParameterType(CDeclManager* decl_manager) {
    if constexpr (I > 1) MakeParameterType<I - 1>();
    ParameterTypes[I] = std::make_unique<TMemberVariable<ParameterCppTypes<I>>>(std::to_string(I), I, this, decl_manager);
  }
  FORCEINLINE TMemberFunction(const std::string& name, asSFuncPtr func_ptr, RClass* owner, CDeclManager* decl_manager)
      : RFunction(name, func_ptr) {
    owner->AddFunction(this);
    decl_manager->RegisterFunction<F>(this);
    if constexpr (!std::is_void_v<ReturnCppType>) {
      ResultType = std::make_unique<TMemberVariable<ReturnCppType>>("r", -1, this, decl_manager);
    }
    if constexpr (ParametersNum > 1) {
      MakeParameterType<ParametersNum - 1>(decl_manager);
    }
  }

  std::unique_ptr<RProperty> ResultType;
  std::array<std::unique_ptr<RProperty>, ParametersNum> ParameterTypes;
};

template <typename F>
struct TGlobalFunction : public RFunction {
  constexpr static size_t ParametersNum = boost::function_types::function_arity<F>::value;
  using ReturnCppType = typename boost::function_types::result_type<F>::type;
  template <size_t I>
  using ParameterCppTypes = typename boost::mpl::at_c<boost::function_types::parameter_types<F>, I>::type;

 protected:
  std::unique_ptr<RProperty> ResultType;
  std::array<std::unique_ptr<RProperty>, ParametersNum> ParameterTypes;

 public:
  template <size_t I>
  FORCEINLINE void MakeParameterType(CDeclManager* decl_manager) {
    if constexpr (I > 0) MakeParameterType<I - 1>(decl_manager);
    ParameterTypes[I] = std::make_unique<TMemberVariable<ParameterCppTypes<I>>>(std::to_string(I), I, this, decl_manager);
  }
  FORCEINLINE void Init(CDeclManager* decl_manager) {
    DeclFlags |= EDeclFlags::Static;
    decl_manager->RegisterFunction<F>(this);
    if constexpr (!std::is_void_v<ReturnCppType>) {
      ResultType = std::make_unique<TMemberVariable<ReturnCppType>>("r", -1, this, decl_manager);
    }
    if constexpr (ParametersNum > 0) {
      MakeParameterType<ParametersNum - 1>(decl_manager);
    }
  }
  FORCEINLINE explicit TGlobalFunction(const std::string& name, asSFuncPtr func_ptr, RClass* owner, CDeclManager* decl_manager)
      : RFunction(name, func_ptr) {
    if (owner) {
      owner->AddFunction(this);
    }
    Init(decl_manager);
  }
  FORCEINLINE explicit TGlobalFunction(const std::string& name, asSFuncPtr func_ptr, RNamespace* owner,
                                       CDeclManager* decl_manager)
      : RFunction(name, func_ptr) {
    if (owner) {
      owner->AddDecl(this);
    }
    Init(decl_manager);
  }
  FORCEINLINE explicit TGlobalFunction(const std::string& name, asSFuncPtr func_ptr, const char* owner,
                                       CDeclManager* decl_manager)
      : RFunction(name, func_ptr) {
    if (owner) {
      if (RNamespace* namespace_owner = decl_manager->GetNamespace(owner)) {
        namespace_owner->AddDecl(this);
      }
    }
    Init(decl_manager);
  }
};

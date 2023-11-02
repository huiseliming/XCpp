#pragma once
#include "DeclManager.h"

#define AS_SUCCEEDED(Expr, ...) X_RUNTIME_ASSERT((Expr) >= 0, ##__VA_ARGS__)

class CScriptManager;
XCORE_API extern CScriptManager& GScriptManager;
XCORE_API CScriptManager& GetScriptManager();

class XCORE_API CScriptManager {
 public:
  CScriptManager();
  ~CScriptManager();
  virtual void Init();
  virtual void Exit();

 protected:
  void RegisterCppDecl();
  void RegisterEnum(REnum* enum_decl);
  void RegisterObjectType(RClass* object_type);
  void RegisterGlobalFunction(RFunction* func_decl);
  void RegisterGlobalProperty(RProperty* prop_decl);
  void SetDefaultNamespace(RDecl* decl);

 protected:
  static void StaticMessageCallback(const asSMessageInfo* msg, void* param);
  void MessageCallback(const asSMessageInfo* msg);

 protected:
  asIScriptEngine* ScriptEngine;
};

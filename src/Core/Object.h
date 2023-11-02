#pragma once
#include "DeclFwd.h"

enum EObjectFlags : uint32 {
  NoneFlagBits,
};

RTYPE()
class XCORE_API OObject {

  OObject() {}
  OObject(asIScriptObject* script_object) : ScriptObject(script_object) {
    ScriptObjectWeakRCFlag = ScriptObject->GetWeakRefFlag();
    ScriptObjectWeakRCFlag->AddRef();
  }
  virtual ~OObject() {
     if (ScriptObjectWeakRCFlag) {
       ScriptObjectWeakRCFlag->Release();
       ScriptObjectWeakRCFlag = nullptr;
     }
  }
 RClass* Class{nullptr};
 OObject* Owner{nullptr};

 protected:
    asIScriptObject* ScriptObject{nullptr};
    asILockableSharedBool* ScriptObjectWeakRCFlag{nullptr};

};

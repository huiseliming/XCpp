#include "ScriptManager.h"
#include <scriptstdstring/scriptstdstring.h>
#include "Object.h"

CScriptManager& GScriptManager = GetScriptManager();

CScriptManager& GetScriptManager() {
  static CScriptManager ScriptManager;
  return ScriptManager;
}

CScriptManager::CScriptManager() {
  X_ASSERT(ScriptEngine = asCreateScriptEngine());
  ScriptEngine->SetMessageCallback(asFUNCTION(StaticMessageCallback), this, asCALL_CDECL);
  RegisterStdString(ScriptEngine);
}

CScriptManager::~CScriptManager() {
  ScriptEngine->ShutDownAndRelease();
}

void* VectorFactory(asITypeInfo* as_type_info) {
  return nullptr;
}

static bool ScriptArrayTemplateCallback(asITypeInfo* ti, bool& dontGarbageCollect) {
  return true;
};

void CScriptManager::Init() {
  ScriptEngine->SetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES, true);
  // ScriptEngine->RegisterTypedef("int32", "int");
  // ScriptEngine->RegisterTypedef("uint32", "uint");

  AS_SUCCEEDED(ScriptEngine->RegisterTypedef("int8_t", "int8"));
  AS_SUCCEEDED(ScriptEngine->RegisterTypedef("int16_t", "int16"));
  AS_SUCCEEDED(ScriptEngine->RegisterTypedef("int32_t", "int"));
  AS_SUCCEEDED(ScriptEngine->RegisterTypedef("int64_t", "int64"));

  AS_SUCCEEDED(ScriptEngine->RegisterTypedef("uint8_t", "uint8"));
  AS_SUCCEEDED(ScriptEngine->RegisterTypedef("uint16_t", "uint16"));
  AS_SUCCEEDED(ScriptEngine->RegisterTypedef("uint32_t", "uint"));
  AS_SUCCEEDED(ScriptEngine->RegisterTypedef("uint64_t", "uint64"));

  RegisterCppDecl();
}

void CScriptManager::Exit() {}

void CScriptManager::RegisterCppDecl() {
  AS_SUCCEEDED(ScriptEngine->SetDefaultNamespace(""));
  AS_SUCCEEDED(ScriptEngine->RegisterObjectType("vector<class T>", 0, asOBJ_REF | asOBJ_NOCOUNT | asOBJ_TEMPLATE));
  AS_SUCCEEDED(
      ScriptEngine->RegisterObjectType("unordered_map<class K, class V>", 0, asOBJ_REF | asOBJ_NOCOUNT | asOBJ_TEMPLATE));
  for (auto& template_specialization_type : g_decl_manager.TemplateSpecializationTypes) {
    if (auto vector_type = dynamic_cast<RVectorType*>(template_specialization_type.get())) {
      SPDLOG_TRACE("ObjectType(vector<{}> , 0, asOBJ_REF | asOBJ_NOCOUNT)", vector_type->EleType->GetFullName());
      AS_SUCCEEDED(ScriptEngine->RegisterObjectType(fmt::format("vector<{}>", vector_type->EleType->GetFullName()).c_str(), 0,
                                                    asOBJ_REF | asOBJ_NOCOUNT));
    } else if (auto unordered_map_type = dynamic_cast<RUnorderedMapType*>(template_specialization_type.get())) {
      SPDLOG_TRACE("ObjectType(unordered_map<{}, {}>, 0, asOBJ_REF | asOBJ_NOCOUNT)",
                   unordered_map_type->KeyType->GetFullName(), unordered_map_type->ValType->GetFullName());
      AS_SUCCEEDED(
          ScriptEngine->RegisterObjectType(fmt::format("unordered_map<{}, {}>", unordered_map_type->KeyType->GetFullName(),
                                                       unordered_map_type->ValType->GetFullName())
                                               .c_str(),
                                           0, asOBJ_REF | asOBJ_NOCOUNT));
    }
  }

  // ScriptEngine->RegisterInterface();
  // ScriptEngine->RegisterEnum();
  for (auto& [name, decl] : g_decl_manager.GetDeclMap()) {
    if (RStruct* struct_decl = dynamic_cast<RStruct*>(decl)) {
      if (RFunction* func_decl = dynamic_cast<RFunction*>(struct_decl)) {
        if (func_decl->IsStatic()) {
          RegisterGlobalFunction(func_decl);
        }
      } else if (RClass* class_decl = dynamic_cast<RClass*>(struct_decl)) {
        if (&GDecl::String != class_decl) {
          RegisterObjectType(class_decl);
        }
      }
    } else if (auto prop_decl = dynamic_cast<RProperty*>(decl)) {
      if (prop_decl->IsStatic()) {
        RegisterGlobalProperty(prop_decl);
      }
    } else if (auto enum_decl = dynamic_cast<REnum*>(decl)) {
      RegisterEnum(enum_decl);
    }
  }
}
void CScriptManager::RegisterEnum(REnum* enum_decl) {
  SetDefaultNamespace(enum_decl);
  SPDLOG_TRACE("Enum({}))", enum_decl->ASDecl.data());
  ScriptEngine->RegisterEnum(enum_decl->ASDecl.data());
  for (auto enum_value : enum_decl->EnumValues) {
    ScriptEngine->RegisterEnumValue(enum_decl->ASDecl.data(), enum_value.Name.data(), enum_value.S32);
  }
}
void CScriptManager::RegisterGlobalFunction(RFunction* func) {
  SetDefaultNamespace(func);
  SPDLOG_TRACE("GlobalFunc({}, *, asCALL_CDECL))", func->ASDecl.data());
  AS_SUCCEEDED(ScriptEngine->RegisterGlobalFunction(func->ASDecl.data(), func->FuncPtr, asCALL_CDECL));
}
void CScriptManager::RegisterGlobalProperty(RProperty* prop) {
  SetDefaultNamespace(prop);
  SPDLOG_TRACE("GlobalProp({}, *)", prop->ASDecl.data());
  AS_SUCCEEDED(ScriptEngine->RegisterGlobalProperty(prop->ASDecl.data(), prop->VariablePtr));
}

void CScriptManager::RegisterObjectType(RClass* object_type) {
  SetDefaultNamespace(object_type);
  if (object_type && object_type->HasAnyDeclFlags(EDeclFlags::Object)) {
    if (object_type->HasAnyDeclFlags(EDeclFlags::RefCntObject)) {
      SPDLOG_TRACE("ObjectType({} , 0, asOBJ_REF)", object_type->GetName());
      AS_SUCCEEDED(ScriptEngine->RegisterObjectType(object_type->GetName().c_str(), 0, asOBJ_REF));
      AS_SUCCEEDED(ScriptEngine->RegisterObjectBehaviour(object_type->GetName().c_str(), asBEHAVE_ADDREF, "void f()",
                                                         asMETHOD(ORefCntObject, AddSharedReference), asCALL_THISCALL));
      AS_SUCCEEDED(ScriptEngine->RegisterObjectBehaviour(object_type->GetName().c_str(), asBEHAVE_RELEASE, "void f()",
                                                         asMETHOD(ORefCntObject, ReleaseSharedReference), asCALL_THISCALL));
    } else {
      SPDLOG_TRACE("ObjectType({} , 0, asOBJ_REF | asOBJ_NOCOUNT)", object_type->GetName());
      AS_SUCCEEDED(ScriptEngine->RegisterObjectType(object_type->GetName().c_str(), 0, asOBJ_REF | asOBJ_NOCOUNT));
    }
  } else {
    SPDLOG_TRACE("ObjectType({} , {}, asOBJ_VALUE)", object_type->GetName(), object_type->GetSize());
    AS_SUCCEEDED(ScriptEngine->RegisterObjectType(object_type->GetName().c_str(), object_type->GetSize(),
                                                  asOBJ_VALUE | object_type->ASTraitsFlag));
    if (object_type->ASTraitsFlag & asOBJ_APP_CLASS_CONSTRUCTOR) {
      AS_SUCCEEDED(ScriptEngine->RegisterObjectBehaviour(object_type->GetName().c_str(), asBEHAVE_CONSTRUCT, "void f()",
                                                         asFUNCTION(object_type->CppTraits.Construct), asCALL_CDECL_OBJFIRST));
    }
    if (object_type->ASTraitsFlag & asOBJ_APP_CLASS_DESTRUCTOR) {
      AS_SUCCEEDED(ScriptEngine->RegisterObjectBehaviour(object_type->GetName().c_str(), asBEHAVE_DESTRUCT, "void f()",
                                                         asFUNCTION(object_type->CppTraits.Destruct), asCALL_CDECL_OBJFIRST));
    }
    if (object_type->ASTraitsFlag & asOBJ_APP_CLASS_COPY_CONSTRUCTOR) {
      AS_SUCCEEDED(ScriptEngine->RegisterObjectBehaviour(
          object_type->GetName().c_str(), asBEHAVE_CONSTRUCT,
          fmt::format("void f(const {} &in)", object_type->GetName(), object_type->GetName()).c_str(),
          asFUNCTION(object_type->CppTraits.CopyConstruct), asCALL_CDECL_OBJFIRST));
    }
    if (object_type->ASTraitsFlag & asOBJ_APP_CLASS_ASSIGNMENT) {
      AS_SUCCEEDED(ScriptEngine->RegisterObjectMethod(
          object_type->GetName().c_str(),
          fmt::format("{} &opAssign(const {} &in)", object_type->GetName(), object_type->GetName()).c_str(),
          asFUNCTION(object_type->CppTraits.CopyAssign), asCALL_CDECL_OBJFIRST));
    }
  }
  // props
  auto prop_it = object_type->EndProp;
  while (prop_it) {
    SPDLOG_TRACE("ObjectProp({}, {}, {})", object_type->GetName().c_str(), prop_it->ASDecl.data(), prop_it->Offset);
    AS_SUCCEEDED(ScriptEngine->RegisterObjectProperty(object_type->GetName().c_str(), prop_it->ASDecl.data(), prop_it->Offset));
    prop_it = prop_it->PrevProp;
  }
  // functions
  if (object_type) {
    auto func_it = object_type->EndFunc;
    while (func_it) {
      if (!func_it->IsStatic()) {
        SPDLOG_TRACE("ObjectFunc({}, {}, *, asCALL_THISCALL))", object_type->GetName(), func_it->ASDecl.data());
        AS_SUCCEEDED(ScriptEngine->RegisterObjectMethod(object_type->GetName().c_str(), func_it->ASDecl.data(),
                                                        func_it->FuncPtr, asCALL_THISCALL));
      }
      func_it = func_it->PrevFunc;
    }
  }
}

void CScriptManager::SetDefaultNamespace(RDecl* decl) {
  if (decl->Owner) {
    AS_SUCCEEDED(ScriptEngine->SetDefaultNamespace(decl->Owner->GetFullName().c_str()));
  } else {
    AS_SUCCEEDED(ScriptEngine->SetDefaultNamespace(""));
  }
}

void CScriptManager::StaticMessageCallback(const asSMessageInfo* msg, void* param) {
  CScriptManager* ScriptManager = reinterpret_cast<CScriptManager*>(param);
  ScriptManager->MessageCallback(msg);
}

void CScriptManager::MessageCallback(const asSMessageInfo* msg) {
  const char* type_to_string[asMSGTYPE_INFORMATION + 1] = {
      "Err ",
      "Warn",
      "Info",
  };
  SPDLOG_ERROR("{:s}({:d}:{:d}) {:s}: {:s}", msg->section, msg->row, msg->col, type_to_string[msg->type], msg->message);
}

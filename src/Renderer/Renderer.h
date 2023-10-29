#pragma once
#include "Utils.h"

struct XRENDERER_API IRenderer {
 protected:
  int32 LastErrorCode;
  std::string LastErrorMessage;

 public:
  int32 GetLastErrorCode() { return LastErrorCode; }
  const std::string& GetLastErrorMessage() { return LastErrorMessage; }

 public:
  virtual void Init() = 0;
  virtual void Render() = 0;
  virtual void Shutdown() = 0;
  virtual IShader* CreateShader(EShaderType shader_type, const char* source, const char* entry_point) = 0;
};
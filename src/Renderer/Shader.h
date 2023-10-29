#pragma once

#include <string>

#include "RHIObject.h"

struct XRENDERER_API IShader : IRHIObject {
 protected:
  EShaderType Type = EShaderType::Undefined;

  FORCEINLINE EShaderType GetType() const { return Type; }

 protected:
  IShader(EShaderType shader_type) : Type(shader_type) {}
};
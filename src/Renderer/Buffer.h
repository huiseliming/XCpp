#pragma once
#include "Utils.h"

struct XRENDERER_API IBuffer : IRHIObject {
 protected:
  EShaderType Type = EShaderType::Undefined;

  FORCEINLINE EShaderType GetType() const { return Type; }

 protected:
  IBuffer() : Type() {}
};
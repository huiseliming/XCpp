#pragma once

#define STBIDEF XRENDERER_API
#include <stb_image.h>

#include "Core/Object.h"

namespace Utils {
XRENDERER_API std::vector<char> LoadBinaryFromFile(const std::string& file_path);
XRENDERER_API std::string LoadCodeFromFile(const std::string& file_path);
}  // namespace Utils

RTYPE()
class OResource : public ORefCntObject
{
 public:
  OResource() {}

};

RTYPE()
class OMesh : public OResource {
 public:
  OMesh() {}
};

RTYPE()
class OTexture : public OResource {
 public:
  OTexture() {}

};

RTYPE()
class OMaterial : public OResource {
 public:
  OMaterial() {}

};


//struct XRENDERER_API IBuffer : IRHIObject {
// protected:
//  EShaderType Type = EShaderType::Undefined;
//
//  FORCEINLINE EShaderType GetType() const { return Type; }
//
// protected:
//  IBuffer() : Type() {}
//};
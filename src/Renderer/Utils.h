#pragma once
#define STBIDEF XRENDERER_API
#include <stb_image.h>

#include "Core/Object.h"

class IRenderer;

namespace Utils {
XRENDERER_API std::vector<char> LoadBinaryFromFile(const std::string& file_path);
XRENDERER_API std::string LoadCodeFromFile(const std::string& file_path);
}  // namespace Utils

struct SVextex
{
  glm::vec3 Position;
  glm::vec2 TextureCoords;
  glm::vec3 Normal;

};

struct SMesh
{
  std::vector<SVextex> Vertices;
  std::vector<uint32> Indices;
  //std::vector<OTexture> Texture;

};

RTYPE()
class OResource : public ORefCntObject {
 public:
  OResource() {}

 protected:
  std::string Name;
};

RTYPE()
class OStaticMesh : public OResource {
 public:
  OStaticMesh() {}
  std::vector<SMesh> Meshes;
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

RTYPE()
class CResourceManager {
 public:
  CResourceManager() = default;
  ~CResourceManager() = default;

 protected:
};

// struct XRENDERER_API IBuffer : IRHIObject {
//  protected:
//   EShaderType Type = EShaderType::Undefined;
//
//   FORCEINLINE EShaderType GetType() const { return Type; }
//
//  protected:
//   IBuffer() : Type() {}
// };
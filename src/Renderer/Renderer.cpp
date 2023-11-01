#include "Renderer.h"
#include <string>
#include <unordered_set>


enum class EResourceType {
  Resource,
  Texture,
  Mesh,
};

std::unordered_map<std::string, EResourceType> ExtensionNameToResourceType = {
    {"collada", EResourceType::Mesh}, {"x", EResourceType::Mesh},        {"stp", EResourceType::Mesh},
    {"obj", EResourceType::Mesh},     {"objnomtl", EResourceType::Mesh}, {"stl", EResourceType::Mesh},
    {"stlb", EResourceType::Mesh},    {"ply", EResourceType::Mesh},      {"plyb", EResourceType::Mesh},
    {"3ds", EResourceType::Mesh},     {"gltf2", EResourceType::Mesh},    {"glb2", EResourceType::Mesh},
    {"gltf", EResourceType::Mesh},    {"glb", EResourceType::Mesh},      {"assbin", EResourceType::Mesh},
    {"assxml", EResourceType::Mesh},  {"x3d", EResourceType::Mesh},      {"fbx", EResourceType::Mesh},
    {"fbxa", EResourceType::Mesh},    {"m3d", EResourceType::Mesh},      {"m3da", EResourceType::Mesh},
    {"3mf", EResourceType::Mesh},     {"pbrt", EResourceType::Mesh},     {"assjson", EResourceType::Mesh},
    {"png", EResourceType::Texture},  {"jpg", EResourceType::Texture},
};

IResource* IRenderer::ImportResourceFromFile(const std::string& file_path) {
  IResource* return_resource = nullptr;
  std::string file_extension = std::filesystem::path(file_path).extension().string();
  auto find_resource_type_it = ExtensionNameToResourceType.find(file_extension);
  if (find_resource_type_it != ExtensionNameToResourceType.end()) {
    switch (find_resource_type_it->second) {
      case EResourceType::Texture:
        return_resource = ImportTextureFromFile(file_path);
        break;
      case EResourceType::Mesh:
        return_resource = ImportMeshFromFile(file_path);
        break;
      default:
        break;
    }
  }
  return return_resource;
}

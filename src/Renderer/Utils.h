#pragma once

namespace Utils {

XRENDERER_API std::vector<char> LoadBinaryFromFile(const std::string& file_path);
XRENDERER_API std::string LoadCodeFromFile(const std::string& file_path);

}  // namespace Utils

enum class EShaderType {
  Undefined,
  Vertex,
  TessControl,
  TessEvaluation,
  Geometry,
  Fragment,
  Compute,
};

struct IShader;
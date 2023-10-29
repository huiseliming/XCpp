#include "GLRenderer.h"

GLenum GetGLShaderType(EShaderType shader_type) {
  switch (shader_type) {
    case EShaderType::Vertex:
      return GL_VERTEX_SHADER;
    case EShaderType::TessControl:
      return GL_TESS_CONTROL_SHADER;
    case EShaderType::TessEvaluation:
      return GL_TESS_EVALUATION_SHADER;
    case EShaderType::Geometry:
      return GL_GEOMETRY_SHADER;
    case EShaderType::Fragment:
      return GL_FRAGMENT_SHADER;
    case EShaderType::Compute:
      return GL_COMPUTE_SHADER;
    default:
      X_ASSERT(false);
  }
  return GLenum();
}

void CGLRenderer::Init() {}

void CGLRenderer::Render() {}

void CGLRenderer::Shutdown() {}

IShader* CGLRenderer::CreateShader(EShaderType shader_type, const char* source, const char* entry_point) {
  GLuint shader_handle = glCreateShader(GetGLShaderType(shader_type));

  glShaderSource(shader_handle, 1, &source, NULL);
  glCompileShader(shader_handle);

  GLint result = GL_FALSE;
  int shader_info_log_length;

  glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &result);
  glGetShaderiv(shader_handle, GL_INFO_LOG_LENGTH, &shader_info_log_length);
  if (shader_info_log_length > 0) {
    std::vector<char> error_message(shader_info_log_length + 1);
    glGetShaderInfoLog(shader_handle, shader_info_log_length, NULL, &error_message[0]);
    printf("%s\n", &error_message[0]);
  }

  return nullptr;
}

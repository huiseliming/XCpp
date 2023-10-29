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
      X_NEVER_EXECUTED();
  }
  return GLenum();
}

void CGLRenderer::Init(SDL_Window* main_window) {
  MainWindow = main_window;
  OpenGLContext = SDL_GL_CreateContext(MainWindow);
  SDL_GL_MakeCurrent(MainWindow, OpenGLContext);
  X_RUNTIME_ASSERT(OpenGLContext != nullptr);

  gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

  X_RUNTIME_ASSERT(SDL_GL_SetSwapInterval(1) >= 0);
}

void CGLRenderer::Render() {
  glClearColor(ClearColor.x, ClearColor.y, ClearColor.z, ClearColor.w);
  glClear(GL_COLOR_BUFFER_BIT);

  SDL_GL_SwapWindow(MainWindow);
}

void CGLRenderer::Shutdown() {
  SDL_GL_DeleteContext(OpenGLContext);
}

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

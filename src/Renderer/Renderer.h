#pragma once
#include "Utils.h"

struct XRENDERER_API IRenderer {
 public:
  virtual ~IRenderer() = default;
  virtual void Init(SDL_Window* main_window) = 0;
  virtual void Render() = 0;
  virtual void Shutdown() = 0;
  virtual IShader* CreateShader(EShaderType shader_type, const char* source, const char* entry_point) = 0;

 public:
  int32 GetLastErrorCode() { return LastErrorCode; }
  const std::string& GetLastErrorMessage() { return LastErrorMessage; }

 protected:
  int32 LastErrorCode;
  std::string LastErrorMessage;

 protected:
  SDL_Window* MainWindow{nullptr};
  glm::vec4 ClearColor = {
      1.0f,
      0.0f,
      0.0f,
      0.0f,
  };
};
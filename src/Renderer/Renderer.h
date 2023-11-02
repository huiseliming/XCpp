#pragma once
#include "Utils.h"

struct XRENDERER_API IRenderer {
 public:
  virtual ~IRenderer() = default;
  virtual void Init(SDL_Window* main_window) = 0;
  virtual void Render() = 0;
  virtual void Shutdown() = 0;

  virtual OResource* ImportResourceFromFile(const std::string& file_path);
  virtual OTexture* ImportTextureFromFile(const std::string& file_path) { return nullptr; }
  virtual OMesh* ImportMeshFromFile(const std::string& file_path) { return nullptr; }

 protected:
  glm::vec3 CameraPosition;
  glm::vec3 CameraForward;
  glm::vec3 CameraUp;

  SDL_Window* MainWindow{nullptr};
  glm::vec4 ClearColor = {
      1.0f,
      0.0f,
      0.0f,
      0.0f,
  };
};
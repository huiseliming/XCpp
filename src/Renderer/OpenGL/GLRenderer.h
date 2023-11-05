#pragma once
#include "GLUtils.h"
#include "../Renderer.h"

struct XRENDERER_API CGLRenderer : public IRenderer {
  virtual void Init(SDL_Window* main_window) override;
  virtual void Render() override;
  virtual void Shutdown() override;

  virtual OTexture* ImportTextureFromFile(const std::string& file_path) override;
  virtual OStaticMesh* ImportMeshFromFile(const std::string& file_path) override;

 protected:
  SDL_GLContext OpenGLContext;

 private:
  friend class CGLImGuiLayer;
};

#pragma once
#include "GLUtils.h"
#include "../Renderer.h"

struct XRENDERER_API CGLRenderer : public IRenderer {
  virtual void Init(SDL_Window* main_window) override;
  virtual void Render() override;
  virtual void Shutdown() override;
  virtual IShader* CreateShader(EShaderType shader_type, const char* source, const char* entry_point) override;

 protected:
  SDL_GLContext OpenGLContext;
};

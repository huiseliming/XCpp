#pragma once
#include "../ImGuiLayer.h"
#include "GLUtils.h"

class XRENDERER_API CGLImGuiLayer : public CImGuiLayer {
 public:
  virtual void Init(SDL_Window* main_window, IRenderer* renderer) override;
  virtual void NewFrame() override;
  virtual void DrawFrame() override;
  virtual void RenderFrame() override;
  virtual void Shutdown() override;

  CGLRenderer* GetGLRenderer() { return reinterpret_cast<CGLRenderer*>(Renderer); }

 protected:

 private:
  friend class CVKRenderer;
};

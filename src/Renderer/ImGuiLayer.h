#pragma once
#include "Utils.h"

class XRENDERER_API CImGuiLayer {
 public:
  virtual ~CImGuiLayer() {}

  virtual void Init(SDL_Window* main_window, IRenderer* renderer) = 0;
  virtual void NewFrame() = 0;
  virtual void DrawFrame();
  virtual void RenderFrame() = 0;
  virtual void Shutdown() = 0;

  static void ProcessEvent(SDL_Event* Event);
  static void HelpMarker(const char* desc);
  void DrawDockSpace();

 protected:

  SDL_Window* MainWindow{nullptr};
  IRenderer* Renderer{nullptr};
  bool bRenderDockSpace;
};

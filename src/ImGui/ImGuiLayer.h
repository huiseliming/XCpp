#pragma once

class XIMGUI_API CImGuiLayer {
 public:
  virtual ~CImGuiLayer() {}

  virtual void CreateContext() {}
  virtual void Init() {}
  virtual void NewFrame() {}
  virtual void DrawFrame() {}
  virtual void RenderFrame() {}
  virtual void Shutdown() {}
  virtual void DestroyContext() {}

  static void ProcessEvent(SDL_Event* Event) {}
  static void HelpMarker(const char* desc) {}

 protected:
  void DrawDockSpace();
};

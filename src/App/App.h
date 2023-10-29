#pragma once

#include <SDL_events.h>
#include "Renderer/Renderer.h"
#include "ImGui/ImGuiLayer.h"

class CApp;

XAPP_API extern CApp* GApp;

enum class ERendererType {
  Null,
  OpenGL,
  Vulkan,
};

class XAPP_API CApp {
 public:
  CApp();
  ~CApp();

 public:
  virtual int Run(int argc = 0, char* argv[] = nullptr);

 protected:
  virtual bool Init();
  virtual void Loop();
  virtual void Exit();

 public:
  SDL_Window* GetMainWindow() { return MainWindow; }
  IRenderer* GetRenderer() { return Renderer; }
  ERendererType GetRendererType() { return RendererType; }

 protected:
  SDL_Window* MainWindow = nullptr;
  IRenderer* Renderer = nullptr;
  CImGuiLayer* ImGuiLayer = nullptr;
  ERendererType RendererType{ERendererType::Vulkan};

 private:
  bool CanExitLoop();
  int ExitCode = 0;
  bool RequiredExitLoop = false;
  std::vector<std::function<bool()>> ExitLoopCheckers;

 private:
  bool ParseArgs(int argc, char* argv[]);
  cxxopts::ParseResult Args;
};

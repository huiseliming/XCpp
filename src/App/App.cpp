#include "App.h"

#include "Renderer/OpenGL/GLRenderer.h"
#include "Renderer/Vulkan/VKRenderer.h"

CApp* GApp = nullptr;

CApp::CApp() {
  GApp = this;
}

CApp::~CApp() {}

int CApp::Run(int argc, char* argv[]) {
  GApp = this;
  try {
#if CK_DEBUG
    spdlog::set_pattern("%Y-%m-%d %H:%M:%S.%e [%t] %s:%# %! %v");
    spdlog::set_level(spdlog::level::trace);
#else
    spdlog::set_pattern("%Y-%m-%d %H:%M:%S.%e [%t] %v");
    spdlog::set_level(spdlog::level::info);
#endif
    SPDLOG_TRACE("");
    if (ParseArgs(argc, argv)) return ExitCode;
    Init();
    while (!CanExitLoop()) {
      Loop();
    }
    Exit();
  } catch (const std::exception& exception) {
    std::cout << "CATCH EXCEPTION: " << exception.what() << std::endl;
    return -1;
  }
  return ExitCode;
}

bool CApp::Init() {
  SPDLOG_TRACE("");
  X_RUNTIME_ASSERT((SDL_Init(SDL_INIT_EVERYTHING) == 0), "SDL_Init Error: {}", SDL_GetError())
  if (Args["vulkan"].as<bool>()) {
    RendererType = ERendererType::Vulkan;
  } else if (Args["opengl"].as<bool>()) {
    RendererType = ERendererType::OpenGL;
  } else {
    RendererType = ERendererType::Null;
  }
  if (RendererType != ERendererType::Null) {
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif
    Uint32 sdl_window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
    if (RendererType == ERendererType::Vulkan) {
      MainWindow = SDL_CreateWindow("App", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720,
                                    sdl_window_flags | SDL_WINDOW_VULKAN);
      Renderer = new CVKRenderer();
    } else if (RendererType == ERendererType::OpenGL) {
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
      SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
      SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
      SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
      MainWindow = SDL_CreateWindow("App", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720,
                                    sdl_window_flags | SDL_WINDOW_OPENGL);
      Renderer = new CGLRenderer();
    } else {
      X_NEVER_EXECUTED();
    }
    Renderer->Init(MainWindow);
  }
  return true;
}

void CApp::Loop() {
  SDL_Event Event;
  while (SDL_PollEvent(&Event)) {
    if (nullptr != Renderer) {
      CImGuiLayer::ProcessEvent(&Event);
    }
    switch (Event.type) {
      case SDL_QUIT:
        bRequiredExitLoop = true;
        break;
      case SDL_KEYDOWN:
        if (Event.key.keysym.sym == SDLK_ESCAPE) {
          bRequiredExitLoop = true;
        }
        break;
      default:
        break;
    }
  }
  if (Renderer) {
    Renderer->Render();
  }
}

void CApp::Exit() {
  SPDLOG_TRACE("");
  if (RendererType != ERendererType::Null) {
    // destroy sdl renderer
    Renderer->Shutdown();
    delete Renderer;
    SDL_DestroyWindow(MainWindow);
  }
  SDL_Quit();
}

bool CApp::CanExitLoop() {
  if (bRequiredExitLoop) {
    ExitLoopCheckers.erase(std::remove_if(ExitLoopCheckers.begin(), ExitLoopCheckers.end(),
                                          [](auto& checker) {
                                            if (checker()) return true;
                                            return false;
                                          }),
                           ExitLoopCheckers.end());
    if (ExitLoopCheckers.empty()) {
      return true;
    }
  }
  return false;
}

bool CApp::ParseArgs(int argc, char* argv[]) {
  cxxopts::Options Options("opts", "parse opts");
  Options.add_options()("console", " ", cxxopts::value<bool>()->default_value("false"))(
      "opengl", "use opengl", cxxopts::value<bool>()->default_value("false"))(
      "vulkan", "use vulkan", cxxopts::value<bool>()->default_value("false"))("version", "print version")("h,help",
                                                                                                          "print usage");
  Args = Options.parse(argc, argv);
  if (Args.count("help")) {
    std::cout << Options.help() << std::endl;
    return true;
  }
  return false;
}

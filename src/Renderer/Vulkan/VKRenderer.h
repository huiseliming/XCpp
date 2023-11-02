#pragma once
#include "VKUtils.h"
#include "../Renderer.h"

struct XRENDERER_API CVKRenderer : public IRenderer {
  virtual void Init(SDL_Window* main_window) override;
  virtual void Render() override;
  virtual void Shutdown() override;

  virtual OTexture* ImportTextureFromFile(const std::string& file_path) override;
  virtual OMesh* ImportMeshFromFile(const std::string& file_path) override;





  vk::Device Device;
};
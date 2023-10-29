#pragma once
#include "VKUtils.h"
#include "../Renderer.h"

struct XRENDERER_API CVKRenderer : public IRenderer {
  virtual void Init() override;
  virtual void Render() override;
  virtual void Shutdown() override;
  virtual IShader* CreateShader(EShaderType shader_type, const char* source, const char* entry_point) override;

  vk::Device Device;
};
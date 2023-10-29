#pragma once
#include "VKUtils.h"
#include "../Shader.h"

struct XRENDERER_API CVKShader : public IShader {
  vk::ShaderModule ShaderModule;
  CVKShader(EShaderType shader_type) : IShader(shader_type) {}
};

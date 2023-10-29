#pragma once
#include "GLUtils.h"
#include "../Shader.h"

struct XRENDERER_API CGLShader : public IShader {
  GLuint ShaderHandle;
  CGLShader(EShaderType shader_type) : IShader(shader_type) {}
};

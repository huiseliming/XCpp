#include "VKRenderer.h"
#include "VKShader.h"

#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Public/resource_limits_c.h>

glslang_stage_t GetGLSLangStageShaderType(EShaderType shader_type) {
  switch (shader_type) {
    case EShaderType::Vertex:
      return GLSLANG_STAGE_VERTEX;
    case EShaderType::TessControl:
      return GLSLANG_STAGE_TESSCONTROL;
    case EShaderType::TessEvaluation:
      return GLSLANG_STAGE_TESSEVALUATION;
    case EShaderType::Geometry:
      return GLSLANG_STAGE_GEOMETRY;
    case EShaderType::Fragment:
      return GLSLANG_STAGE_FRAGMENT;
    case EShaderType::Compute:
      return GLSLANG_STAGE_COMPUTE;
    default:
      X_NEVER_EXECUTED();
  }
  return glslang_stage_t();
}

void CVKRenderer::Init(SDL_Window* main_window) {}

void CVKRenderer::Render() {}

void CVKRenderer::Shutdown() {}

IShader* CVKRenderer::CreateShader(EShaderType shader_type, const char* source, const char* entry_point) {
  std::string shader_code = Utils::LoadCodeFromFile(source);
  const glslang_input_t input = {.language = GLSLANG_SOURCE_GLSL,
                                 .stage = GetGLSLangStageShaderType(shader_type),
                                 .client = GLSLANG_CLIENT_VULKAN,
                                 .client_version = GLSLANG_TARGET_VULKAN_1_1,
                                 .target_language = GLSLANG_TARGET_SPV,
                                 .target_language_version = GLSLANG_TARGET_SPV_1_3,
                                 .code = shader_code.c_str(),
                                 .default_version = 460,
                                 .default_profile = GLSLANG_CORE_PROFILE,
                                 .force_default_version_and_profile = 0,
                                 .forward_compatible = 0,
                                 .messages = GLSLANG_MSG_DEFAULT_BIT,
                                 .resource = glslang_default_resource()};
  if (glslang_shader_t* glslang_shader = glslang_shader_create(&input)) {
    if (glslang_shader_preprocess(glslang_shader, &input)) {
      if (!glslang_shader_parse(glslang_shader, &input)) {
        if (glslang_program_t* glslang_program = glslang_program_create()) {
          glslang_program_add_shader(glslang_program, glslang_shader);
          if (glslang_program_link(glslang_program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT)) {
            glslang_program_SPIRV_generate(glslang_program, input.stage);
            if (auto messages = glslang_program_SPIRV_get_messages(glslang_program)) {
              SPDLOG_WARN("glslang_program_SPIRV_get_messages : {}", messages);
            }
            vk::ShaderModuleCreateInfo shader_module_create_info;
            shader_module_create_info.codeSize = glslang_program_SPIRV_get_size(glslang_program) * sizeof(unsigned int);
            shader_module_create_info.pCode = glslang_program_SPIRV_get_ptr(glslang_program);
            auto ShaderModuleResult = Device.createShaderModule(shader_module_create_info);
            VK_SUCCEEDED(ShaderModuleResult.result);
            CVKShader* shader = new CVKShader(shader_type);
            shader->ShaderModule = ShaderModuleResult.value;
            return shader;
          } else {
            LastErrorCode = -1;
            LastErrorMessage = fmt::format("glslang_program_link : {} ({})!", glslang_shader_get_info_log(glslang_shader),
                                           glslang_shader_get_info_debug_log(glslang_shader));
          }
          glslang_program_delete(glslang_program);
        } else {
          LastErrorCode = -1;
          LastErrorMessage = fmt::format("glslang_program_create : {} ({})!", glslang_shader_get_info_log(glslang_shader),
                                         glslang_shader_get_info_debug_log(glslang_shader));
        }
      } else {
        LastErrorCode = -1;
        LastErrorMessage = fmt::format("glslang_shader_parse : {} ({})!", glslang_shader_get_info_log(glslang_shader),
                                       glslang_shader_get_info_debug_log(glslang_shader));
      }
    } else {
      LastErrorCode = -1;
      LastErrorMessage = fmt::format("glslang_shader_preprocess : {} ({})!", glslang_shader_get_info_log(glslang_shader),
                                     glslang_shader_get_info_debug_log(glslang_shader));
    }
    glslang_shader_delete(glslang_shader);
  } else {
    LastErrorCode = -1;
    LastErrorMessage = fmt::format("glslang_shader_create : {} ({})!", glslang_shader_get_info_log(glslang_shader),
                                   glslang_shader_get_info_debug_log(glslang_shader));
  }
  return nullptr;
}

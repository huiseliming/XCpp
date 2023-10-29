#pragma once
#include "../Utils.h"
#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_ASSERT_ON_RESULT
#include <vulkan/vulkan.hpp>

#define VK_SUCCEEDED(Expr) \
  X_RUNTIME_ASSERT((Expr) == vk::Result::eSuccess, "vk::Result : {}", vk::to_string(vk::Result::eSuccess))

class CVKRenderer;
class CVKShader;
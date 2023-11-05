#pragma once

#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_ASSERT_ON_RESULT
#include <vulkan/vulkan.hpp>

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#define VMA_CALL_PRE XRENDERER_API
#include "vk_mem_alloc.h"

#include <SDL_vulkan.h>
#include "../Utils.h"


#define VK_SUCCEEDED(Expr) \
  X_RUNTIME_ASSERT((Expr) == vk::Result::eSuccess, "vk::Result : {}", vk::to_string(vk::Result::eSuccess))

struct CVKRenderer;


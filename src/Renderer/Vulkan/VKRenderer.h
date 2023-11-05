#pragma once
#include "VKUtils.h"
#include "../Renderer.h"
#include "VKImGuiLayer.h"

struct SCreateBufferResult {
  VkBuffer Buffer;
  VmaAllocation Allocation;
  VmaAllocationInfo AllocationInfo;
};

struct SCreateImageResult {
  VkImage Image;
  VmaAllocation Allocation;
  VmaAllocationInfo AllocationInfo;
};

struct XRENDERER_API CVKRenderer : public IRenderer {
  virtual void Init(SDL_Window* main_window) override;
  virtual void Render() override;
  virtual void Shutdown() override;
  virtual OTexture* ImportTextureFromFile(const std::string& file_path) override;
  virtual OStaticMesh* ImportMeshFromFile(const std::string& file_path) override;

  std::function<void()> DrawWorld;
  std::function<void()> DrawUI;

  void RebuildSwapchain();

 protected:
  void CreateInstance();
  void CreateSurface();
  void CreateDevice();
  void CreateMemoryAllocator();
  void CreateSwapchain();
  void CreateRenderPass();
  void CreateFramebuffers();
  void CreateSyncObjects();
  void CreateCommandPool();
  void AllocateCommandBuffers();

  void RecreateSwapchain();

  void DestroyCommandPool();
  void DestroySyncObjects();
  void DestroyFramebuffers();
  void DestroyRenderPass();
  void DestroySwapchain();
  void DestroyMemoryAllocator();
  void DestroyDevice();
  void DestroySurface();
  void DestroyInstance();

  CVKImGuiLayer* GetVKImGuiLayer() { return static_cast<CVKImGuiLayer*>(ImGuiLayer); }

 protected:
  FORCEINLINE vk::CommandBuffer AllocateAndBeginCommandBuffer() {
    vk::CommandBuffer command_buffer;
    vk::CommandBufferAllocateInfo CommandBufferAllocateInfo;
    CommandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
    CommandBufferAllocateInfo.commandPool = CommandPool;
    CommandBufferAllocateInfo.commandBufferCount = 1;

    auto CommandBuffersResult = Device.allocateCommandBuffers(CommandBufferAllocateInfo);
    VK_SUCCEEDED(CommandBuffersResult.result);
    command_buffer = CommandBuffersResult.value[0];

    vk::CommandBufferBeginInfo CommandBufferBeginInfo;
    CommandBufferBeginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    VK_SUCCEEDED(command_buffer.begin(CommandBufferBeginInfo));
    return command_buffer;
  }
  FORCEINLINE void SubmitCommandBufferWaitIdle(vk::CommandBuffer command_buffer) {
    VK_SUCCEEDED(command_buffer.end());
    vk::SubmitInfo SubmitInfo;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &command_buffer;
    VK_SUCCEEDED(GraphicsQueue.submit(SubmitInfo));
    // VK_SUCCEEDED(VulkanRenderer->Device.waitIdle());
    VK_SUCCEEDED(GraphicsQueue.waitIdle());
    Device.freeCommandBuffers(CommandPool, command_buffer);
  }

  FORCEINLINE SCreateBufferResult CreateBuffer(vk::BufferCreateInfo buffer_ci, VmaAllocationCreateFlags allocation_create_flags,
                                               VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_AUTO) {
    VmaAllocationCreateInfo allocation_ci = {};
    allocation_ci.flags = allocation_create_flags;
    allocation_ci.usage = memory_usage;

    SCreateBufferResult result;
    X_ASSERT(VK_SUCCESS == vmaCreateBuffer(MemoryAllocator, &buffer_ci.operator VkBufferCreateInfo&(), &allocation_ci,
                                           &result.Buffer, &result.Allocation, nullptr));
    return result;
  }

  FORCEINLINE SCreateImageResult CreateImage(vk::ImageCreateInfo image_ci, VmaAllocationCreateFlags allocation_create_flags,
                                             VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_AUTO) {
    VmaAllocationCreateInfo allocation_ci = {};
    allocation_ci.flags = allocation_create_flags;
    allocation_ci.usage = memory_usage;

    SCreateImageResult result;
    X_ASSERT(VK_SUCCESS == vmaCreateImage(MemoryAllocator, &image_ci.operator VkImageCreateInfo&(), &allocation_ci,
                                          &result.Image, &result.Allocation, nullptr));

    return result;
  }
  FORCEINLINE SCreateBufferResult CreateTransferSrc(vk::DeviceSize size) {
    vk::BufferCreateInfo buffer_ci;
    buffer_ci.flags = {};
    buffer_ci.size = size;
    buffer_ci.usage = vk::BufferUsageFlagBits::eTransferSrc;
    buffer_ci.sharingMode = VULKAN_HPP_NAMESPACE::SharingMode::eExclusive;
    buffer_ci.queueFamilyIndexCount = {};
    const uint32_t* pQueueFamilyIndices = {};
    return CreateBuffer(buffer_ci, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);
  }

  FORCEINLINE void CopyBufferToImage(vk::Buffer buffer, vk::Image image, uint32 image_width, uint32 image_height) {
    auto command_buffer = AllocateAndBeginCommandBuffer();

    vk::BufferImageCopy buffer_image_copy;
    buffer_image_copy.bufferOffset = 0;
    buffer_image_copy.bufferRowLength = 0;
    buffer_image_copy.bufferImageHeight = 0;
    buffer_image_copy.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    buffer_image_copy.imageSubresource.mipLevel = 0;
    buffer_image_copy.imageSubresource.baseArrayLayer = 0;
    buffer_image_copy.imageSubresource.layerCount = 1;
    buffer_image_copy.imageOffset = vk::Offset3D(0, 0, 0);
    buffer_image_copy.imageExtent = vk::Extent3D(image_width, image_height, 1);
    command_buffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, buffer_image_copy);
    SubmitCommandBufferWaitIdle(command_buffer);
  }
  FORCEINLINE void TransitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout old_image_layout,
                                         vk::ImageLayout new_image_layout) {
    auto command_buffer = AllocateAndBeginCommandBuffer();
    vk::ImageMemoryBarrier image_memory_barrier;
    image_memory_barrier.srcAccessMask = {};
    image_memory_barrier.dstAccessMask = {};
    image_memory_barrier.oldLayout = old_image_layout;
    image_memory_barrier.newLayout = new_image_layout;
    image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.image = image;
    image_memory_barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    image_memory_barrier.subresourceRange.baseMipLevel = 0;
    image_memory_barrier.subresourceRange.levelCount = 1;
    image_memory_barrier.subresourceRange.baseArrayLayer = 0;
    image_memory_barrier.subresourceRange.layerCount = 1;
    vk::PipelineStageFlags src_pipeline_stage_flags;
    vk::PipelineStageFlags dst_pipeline_stage_flags;
    if (old_image_layout == vk::ImageLayout::eUndefined && new_image_layout == vk::ImageLayout::eTransferDstOptimal) {
      image_memory_barrier.srcAccessMask = vk::AccessFlagBits::eNone;
      image_memory_barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
      src_pipeline_stage_flags = vk::PipelineStageFlagBits::eTopOfPipe;
      dst_pipeline_stage_flags = vk::PipelineStageFlagBits::eTransfer;
    } else if (old_image_layout == vk::ImageLayout::eTransferDstOptimal &&
               new_image_layout == vk::ImageLayout::eShaderReadOnlyOptimal) {
      image_memory_barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
      image_memory_barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
      src_pipeline_stage_flags = vk::PipelineStageFlagBits::eTransfer;
      dst_pipeline_stage_flags = vk::PipelineStageFlagBits::eFragmentShader;
    } else {
      X_ASSERT(false);
    }
    command_buffer.pipelineBarrier(src_pipeline_stage_flags, dst_pipeline_stage_flags, vk::DependencyFlags(), 0, nullptr, 0,
                                   nullptr, 1, &image_memory_barrier);
    SubmitCommandBufferWaitIdle(command_buffer);
  }

 protected:
  FORCEINLINE vk::SurfaceFormatKHR ChooseSwapchainSurfaceFormat();
  FORCEINLINE vk::PresentModeKHR ChooseSwapchainPresentMode();
  FORCEINLINE vk::Extent2D ChooseSwapchainExtent();

  FORCEINLINE bool IsLayerSupported(const std::string& InLayerName);
  FORCEINLINE bool IsExtensionSupported(const std::string& InExtensionName);

  FORCEINLINE uint32 FindMemoryTypeIndex(uint32 InMemoryTypeBits /* SupportetMemoryIndices */,
                                         vk::MemoryPropertyFlags InRequestedMemoryPropertyFlags);

 protected:
  std::vector<std::string> SupportedLayers;
  std::vector<std::string> SupportedExtensions;
  std::vector<const char*> EnabledLayers;
  std::vector<const char*> EnabledExtensions;
  bool bEnableValidationLayers = true;
  bool bUseDebugMessenger = true;
  bool bPortabilityEnumerationSupport = false;

  vk::Instance Instance;
  vk::DispatchLoaderDynamic DispatchLoaderDynamic;
  vk::DebugUtilsMessengerEXT DebugUtilsMessenger;

  vk::SurfaceKHR Surface;

  vk::PhysicalDevice PhysicalDevice;
  uint32 GraphicsFamilyIndex = -1;
  uint32 PresentFamilyIndex = -1;

  vk::Device Device;
  vk::Queue GraphicsQueue;
  vk::Queue PresentQueue;

  VmaAllocator MemoryAllocator;

  uint32 MinImageCount;
  vk::SurfaceCapabilitiesKHR SurfaceCapabilities;
  std::vector<vk::SurfaceFormatKHR> SurfaceFormats;
  std::vector<vk::PresentModeKHR> PresentModes;

  vk::SwapchainKHR Swapchain;
  vk::SurfaceFormatKHR SwapchainSurfaceFormat;
  vk::PresentModeKHR SwapchainPresentMode;
  vk::Extent2D SwapchainExtent;
  bool bRequireRecreateSwapchain = false;

  vk::RenderPass RenderPass;

  struct SRenderFrame {
    vk::Image SwapchainImage;
    vk::ImageView SwapchainImageView;
    vk::Semaphore ImageAvailableSemaphore;
    vk::Semaphore RenderFinishedSemaphore;
    vk::Fence InFlightFence;
    vk::Framebuffer Framebuffer;
    vk::CommandBuffer CommandBuffer;
  };

  std::vector<SRenderFrame> RenderFrames;
  uint32 MaxFramesInFlight;
  uint32 RenderFrameIndex;

  vk::CommandPool CommandPool;
  vk::CommandBuffer MainCommandBuffer;

 private:
  static VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                         VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                         const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                         void* pUserData);
  friend class CVKImGuiLayer;
  friend class CVKScopeCommand;
};

vk::SurfaceFormatKHR CVKRenderer::ChooseSwapchainSurfaceFormat() {
  for (auto SurfaceFormat : SurfaceFormats) {
    if (SurfaceFormat.format == vk::Format::eB8G8R8A8Unorm && SurfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      return SurfaceFormat;
    }
  }
  return SurfaceFormats[0];
}

vk::PresentModeKHR CVKRenderer::ChooseSwapchainPresentMode() {
  for (auto PresentMode : PresentModes) {
    if (PresentMode == vk::PresentModeKHR::eMailbox) {
      return PresentMode;
    }
  }
  return vk::PresentModeKHR::eFifo;
}

vk::Extent2D CVKRenderer::ChooseSwapchainExtent() {
  int32 Width, Height;
  SDL_GetWindowSize(MainWindow, &Width, &Height);
  vk::Extent2D SwapchainExtent{static_cast<uint32>(Width), static_cast<uint32>(Height)};
  if (SurfaceCapabilities.currentExtent.width != UINT32_MAX) {
    return SurfaceCapabilities.currentExtent;
  } else {
    vk::Extent2D Extent = SwapchainExtent;
    Extent.width = (std::min)(SurfaceCapabilities.maxImageExtent.width,
                              (std::max)(SurfaceCapabilities.minImageExtent.width, static_cast<uint32>(SwapchainExtent.width)));
    Extent.height = (std::min)(
        SurfaceCapabilities.maxImageExtent.height,
        (std::max)(SurfaceCapabilities.minImageExtent.height, static_cast<uint32>(SwapchainExtent.height)));
    return Extent;
  }
}

bool CVKRenderer::IsLayerSupported(const std::string& InLayerName) {
  for (auto& SupportedLayer : SupportedLayers) {
    if (InLayerName == SupportedLayer) {
      return true;
    }
  }
  return false;
}

bool CVKRenderer::IsExtensionSupported(const std::string& InExtensionName) {
  for (auto& SupportedExtension : SupportedExtensions) {
    if (InExtensionName == SupportedExtension) {
      return true;
    }
  }
  return false;
}

uint32 CVKRenderer::FindMemoryTypeIndex(uint32 InMemoryTypeBits /* SupportetMemoryIndices */,
                                        vk::MemoryPropertyFlags InRequestedMemoryPropertyFlags) {
  vk::PhysicalDeviceMemoryProperties MemoryProperties = PhysicalDevice.getMemoryProperties();
  for (uint32 i = 0; i < MemoryProperties.memoryTypeCount; i++) {
    if ((InMemoryTypeBits & (1 << i)) &&
        (MemoryProperties.memoryTypes[i].propertyFlags & InRequestedMemoryPropertyFlags) == InRequestedMemoryPropertyFlags) {
      return i;
    }
  }
  X_RUNTIME_ASSERT(false);
  return std::numeric_limits<uint32>::max();
}
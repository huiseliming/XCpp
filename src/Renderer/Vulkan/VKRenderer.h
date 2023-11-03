#pragma once
#include "VKUtils.h"
#include "../Renderer.h"
#include "VKImGuiLayer.h"

class CVKScopeCommand {
 public:
  FORCEINLINE CVKScopeCommand(CVKRenderer* vk_renderer) : VKRenderer(vk_renderer) { SingleCommandBegin(); }
  CVKScopeCommand(const CVKScopeCommand&) = delete;
  FORCEINLINE CVKScopeCommand(CVKScopeCommand&& Other)
      : VKRenderer(Other.VKRenderer), SingleCommandBuffer(Other.SingleCommandBuffer) {
    Other.SingleCommandBuffer = vk::CommandBuffer();
  }
  CVKScopeCommand& operator=(const CVKScopeCommand&) = delete;
  CVKScopeCommand& operator=(CVKScopeCommand&&) = delete;
  FORCEINLINE ~CVKScopeCommand() { SingleCommandEnd(); }
  operator VkCommandBuffer() { return SingleCommandBuffer; }
  operator vk::CommandBuffer() { return SingleCommandBuffer; }

 protected:
  FORCEINLINE void SingleCommandBegin();
  FORCEINLINE void SingleCommandEnd();

 private:
  vk::CommandBuffer SingleCommandBuffer;
  CVKRenderer* VKRenderer;

  friend class CVulkanRenderer;
};

struct XRENDERER_API CVKRenderer : public IRenderer {
  virtual void Init(SDL_Window* main_window) override;
  virtual void Render() override;
  virtual void Shutdown() override;
  virtual OTexture* ImportTextureFromFile(const std::string& file_path) override;
  virtual OMesh* ImportMeshFromFile(const std::string& file_path) override;
  
  std::function<void()> DrawWorld;
  std::function<void()> DrawUI;

  void RebuildSwapchain();

 protected:
  void CreateInstance();
  void CreateSurface();
  void CreateDevice();
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
  void DestroyDevice();
  void DestroySurface();
  void DestroyInstance();

  CVKImGuiLayer* GetVKImGuiLayer() { return static_cast<CVKImGuiLayer*>(ImGuiLayer); }

 protected:
  FORCEINLINE CVKScopeCommand CreateSingleScopeCommand();
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

  struct SRenderFrame
  {
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

void CVKScopeCommand::SingleCommandBegin() {
  vk::CommandBufferAllocateInfo CommandBufferAllocateInfo;
  CommandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
  CommandBufferAllocateInfo.commandPool = VKRenderer->CommandPool;
  CommandBufferAllocateInfo.commandBufferCount = 1;

  auto CommandBuffersResult = VKRenderer->Device.allocateCommandBuffers(CommandBufferAllocateInfo);
  VK_SUCCEEDED(CommandBuffersResult.result);
  SingleCommandBuffer = CommandBuffersResult.value[0];

  vk::CommandBufferBeginInfo CommandBufferBeginInfo;
  CommandBufferBeginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
  VK_SUCCEEDED(SingleCommandBuffer.begin(CommandBufferBeginInfo));
}
void CVKScopeCommand::SingleCommandEnd() {
  if (SingleCommandBuffer) {
    VK_SUCCEEDED(SingleCommandBuffer.end());

    vk::SubmitInfo SubmitInfo;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &SingleCommandBuffer;
    VK_SUCCEEDED(VKRenderer->GraphicsQueue.submit(SubmitInfo));
    // VK_SUCCEEDED(VulkanRenderer->Device.waitIdle());
    VK_SUCCEEDED(VKRenderer->GraphicsQueue.waitIdle());
    VKRenderer->Device.freeCommandBuffers(VKRenderer->CommandPool, SingleCommandBuffer);
  }
}

FORCEINLINE CVKScopeCommand CVKRenderer::CreateSingleScopeCommand() {
  return CVKScopeCommand(this);
}

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
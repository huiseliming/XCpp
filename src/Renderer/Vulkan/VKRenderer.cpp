#include "VKRenderer.h"

#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Public/resource_limits_c.h>

void CVKRenderer::Init(SDL_Window* main_window) {
  MainWindow = main_window;
  CreateInstance();
  CreateSurface();
  CreateDevice();
  CreateSwapchain();
  CreateRenderPass();
  CreateFramebuffers();
  CreateSyncObjects();
  CreateCommandPool();
  AllocateCommandBuffers();
  ImGuiLayer = new CVKImGuiLayer();
  ImGuiLayer->Init(MainWindow, this);
}

void CVKRenderer::Render() {
  SRenderFrame& RenderFrame = RenderFrames[RenderFrameIndex];
  VK_SUCCEEDED(Device.waitForFences(1, &RenderFrame.InFlightFence, VK_TRUE, UINT64_MAX));
  auto ImageIndexResult = Device.acquireNextImageKHR(Swapchain, UINT64_MAX, RenderFrame.ImageAvailableSemaphore, nullptr);
  if (ImageIndexResult.result != vk::Result::eSuccess) {
    bRequireRecreateSwapchain = true;
    return;
  }
  uint32 ImageIndex = ImageIndexResult.value;
  X_ASSERT(RenderFrameIndex == ImageIndex);
  // Render Command BG
  RenderFrame.CommandBuffer.reset();
  vk::CommandBuffer CommandBuffer = RenderFrame.CommandBuffer;
  vk::CommandBufferBeginInfo CommandBufferBeginInfo;
  VK_SUCCEEDED(CommandBuffer.begin(CommandBufferBeginInfo));

  vk::RenderPassBeginInfo RenderPassBeginInfo;
  RenderPassBeginInfo.renderPass = RenderPass;
  RenderPassBeginInfo.framebuffer = RenderFrame.Framebuffer;
  RenderPassBeginInfo.renderArea.offset.x = 0;
  RenderPassBeginInfo.renderArea.offset.y = 0;
  RenderPassBeginInfo.renderArea.extent = SwapchainExtent;
  vk::ClearValue clear_color = vk::ClearColorValue{ClearColor.x, ClearColor.y, ClearColor.z, ClearColor.w};
  RenderPassBeginInfo.clearValueCount = 1;
  RenderPassBeginInfo.pClearValues = &clear_color;
  CommandBuffer.beginRenderPass(&RenderPassBeginInfo, vk::SubpassContents::eInline);
  // CommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, PipelineLayout, 0, RenderFrame.DescriptorSet, nullptr);
  // CommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, Pipeline);
  //// CommandBuffer.draw(3, 1, 0, 0);
  // DrawWorld(InImageIndex);
  CommandBuffer.endRenderPass();
  VK_SUCCEEDED(CommandBuffer.end());
  // Render Command ED
  // 
  // ImGui Render Command BG
    ImGuiLayer->NewFrame();
    ImGuiLayer->DrawFrame();
    ImGuiLayer->RenderFrame();
  // ImGui Render Command ED

  vk::Semaphore RenderWaitSemaphores[] = {RenderFrame.ImageAvailableSemaphore};
  vk::PipelineStageFlags WaitDstStage = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
  
  vk::CommandBuffer RenderCommandBuffers[] = {RenderFrame.CommandBuffer, GetVKImGuiLayer()->RenderFrames[RenderFrameIndex].CommandBuffer};

  vk::Semaphore SignalSemaphores[] = {RenderFrame.RenderFinishedSemaphore};

  vk::SubmitInfo SubmitInfo;
  SubmitInfo.waitSemaphoreCount = std::size(RenderWaitSemaphores);
  SubmitInfo.pWaitSemaphores = RenderWaitSemaphores;
  SubmitInfo.pWaitDstStageMask = &WaitDstStage;
  SubmitInfo.commandBufferCount = std::size(RenderCommandBuffers);
  SubmitInfo.pCommandBuffers = RenderCommandBuffers;
  SubmitInfo.signalSemaphoreCount = std::size(SignalSemaphores);
  SubmitInfo.pSignalSemaphores = SignalSemaphores;
  VK_SUCCEEDED(Device.resetFences(1, &RenderFrame.InFlightFence));
  VK_SUCCEEDED(GraphicsQueue.submit(SubmitInfo, RenderFrame.InFlightFence));

  vk::SwapchainKHR Swapchains[] = {Swapchain};
  uint32 ImageIndices[] = {ImageIndex};
  vk::Result PresentResult = vk::Result::eSuccess;

  vk::PresentInfoKHR PresentInfo;
  PresentInfo.waitSemaphoreCount = std::size(SignalSemaphores);
  PresentInfo.pWaitSemaphores = SignalSemaphores;
  PresentInfo.swapchainCount = std::size(Swapchains);
  ;
  PresentInfo.pSwapchains = Swapchains;
  PresentInfo.pImageIndices = ImageIndices;
  // PresentInfo.pResults = &PresentResult;
  PresentResult = PresentQueue.presentKHR(PresentInfo);
  if (PresentResult == vk::Result::eErrorOutOfDateKHR || PresentResult == vk::Result::eSuboptimalKHR) {
    bRequireRecreateSwapchain = true;
    return;
  }
  RenderFrameIndex = (RenderFrameIndex + 1) % MaxFramesInFlight;
}

void CVKRenderer::Shutdown() {
  VK_SUCCEEDED(Device.waitIdle());
  ImGuiLayer->Shutdown();
  delete ImGuiLayer;
  DestroyCommandPool();
  DestroySyncObjects();
  DestroyFramebuffers();
  DestroyRenderPass();
  DestroySwapchain();
  DestroyDevice();
  DestroySurface();
  DestroyInstance();
}

OTexture* CVKRenderer::ImportTextureFromFile(const std::string& file_path) {

  return nullptr;
}

OMesh* CVKRenderer::ImportMeshFromFile(const std::string& file_path) {
  return nullptr;
}

void CVKRenderer::CreateInstance() {
  uint32_t InstanceExtensionsCount = 0;
  std::vector<const char*> InstanceExtensions;
  SDL_Vulkan_GetInstanceExtensions(MainWindow, &InstanceExtensionsCount, nullptr);
  InstanceExtensions.resize(InstanceExtensionsCount);
  SDL_Vulkan_GetInstanceExtensions(MainWindow, &InstanceExtensionsCount, InstanceExtensions.data());

  EnabledExtensions.insert(EnabledExtensions.end(), InstanceExtensions.begin(), InstanceExtensions.end());

  std::unordered_set<std::string> SupportedExtensionSet;
  uint32 VulkanInstanceVersion = 0;
  VK_SUCCEEDED(vk::enumerateInstanceVersion(&VulkanInstanceVersion));
  SPDLOG_ERROR("VulkanInstanceVersion : {}.{}.{}", VK_API_VERSION_MAJOR(VulkanInstanceVersion),
               VK_API_VERSION_MINOR(VulkanInstanceVersion), VK_API_VERSION_PATCH(VulkanInstanceVersion));
  {
    SPDLOG_ERROR("Support Extensions :");
    vk::ResultValue<std::vector<vk::ExtensionProperties>> SupportedExtensionsResult =
        vk::enumerateInstanceExtensionProperties();
    VK_SUCCEEDED(SupportedExtensionsResult.result);
    for (auto& SupportedExtension : SupportedExtensionsResult.value) {
      SupportedExtensionSet.insert(SupportedExtension.extensionName);
#ifndef NDEBUG
      SPDLOG_ERROR("    Extension : {}", SupportedExtension.extensionName.data());
#endif
    }
  }

  SPDLOG_ERROR("Support Layers :");
  vk::ResultValue<std::vector<vk::LayerProperties>> SupportedLayersResult = vk::enumerateInstanceLayerProperties();
  VK_SUCCEEDED(SupportedLayersResult.result);
  for (auto& SupportedLayer : SupportedLayersResult.value) {
    SupportedLayers.push_back(SupportedLayer.layerName);
#ifndef NDEBUG
    SPDLOG_ERROR("    Layer : {}", SupportedLayer.layerName.data());
#endif
    vk::ResultValue<std::vector<vk::ExtensionProperties>> SupportedExtensionsResult = vk::enumerateInstanceExtensionProperties(
        SupportedLayers.back());
    VK_SUCCEEDED(SupportedExtensionsResult.result);
    for (auto& SupportedExtension : SupportedExtensionsResult.value) {
      SupportedExtensionSet.insert(SupportedExtension.extensionName);
#ifndef NDEBUG
      SPDLOG_ERROR("        Extension : {}", SupportedExtension.extensionName.data());
#endif
    }
  }

  for (auto& SupportedExtension : SupportedExtensionSet) {
    SupportedExtensions.push_back(SupportedExtension);
  }

  if (bEnableValidationLayers) {
    EnabledLayers.push_back("VK_LAYER_KHRONOS_validation");
#if VK_EXT_validation_features
    EnabledExtensions.push_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
#endif
  }

  if (bUseDebugMessenger && IsExtensionSupported(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
    EnabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

#if defined(VK_KHR_portability_enumeration)
  if (IsExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
    EnabledExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
  }
  bool bIsPortabilityEnumerationSupport = IsExtensionSupported(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
  if (bIsPortabilityEnumerationSupport) {
    EnabledExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
  }
#else
  bool bIsPortabilityEnumerationSupport = false;
#endif

  SPDLOG_ERROR("Enabled Layers :");
  for (auto& EnabledLayer : EnabledLayers) {
    SPDLOG_ERROR("    Layer : {}", EnabledLayer);
    X_RUNTIME_ASSERT(IsLayerSupported(EnabledLayer));
  }

  SPDLOG_ERROR("Enabled Extensions :");
  for (auto& EnabledExtension : EnabledExtensions) {
    SPDLOG_ERROR("    Extensions : {}", EnabledExtension);
    X_RUNTIME_ASSERT(IsExtensionSupported(EnabledExtension));
  }

  vk::ApplicationInfo ApplicationInfo;
  ApplicationInfo.setPApplicationName("");
  ApplicationInfo.setApplicationVersion(1);
  ApplicationInfo.setPEngineName("MiniEngine");
  ApplicationInfo.setEngineVersion(0);
  ApplicationInfo.setApiVersion(VK_API_VERSION_1_1);

  vk::InstanceCreateInfo InstanceCreateInfo;
#if VK_EXT_validation_features
  if (bEnableValidationLayers) {
    vk::ValidationFeatureEnableEXT ValidationFeatureEnableEXT[] = {vk::ValidationFeatureEnableEXT::eDebugPrintf};
    vk::ValidationFeaturesEXT ValidationFeaturesEXT = {};
    ValidationFeaturesEXT.enabledValidationFeatureCount = 1;
    ValidationFeaturesEXT.pEnabledValidationFeatures = ValidationFeatureEnableEXT;
    InstanceCreateInfo.pNext = &ValidationFeaturesEXT;
  }
#endif
#if defined(VK_KHR_portability_enumeration)
  if (bIsPortabilityEnumerationSupport) {
    InstanceCreateInfo.setFlags(InstanceCreateInfo.flags | vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR);
  }
#endif
  InstanceCreateInfo.setPApplicationInfo(&ApplicationInfo);
  InstanceCreateInfo.setEnabledLayerCount(EnabledLayers.size());
  InstanceCreateInfo.setPpEnabledLayerNames(EnabledLayers.data());
  InstanceCreateInfo.setEnabledExtensionCount(EnabledExtensions.size());
  InstanceCreateInfo.setPpEnabledExtensionNames(EnabledExtensions.data());
  vk::ResultValue<vk::Instance> InstanceResult = vk::createInstance(InstanceCreateInfo);
  VK_SUCCEEDED(InstanceResult.result);
  Instance = InstanceResult.value;
  DispatchLoaderDynamic = vk::DispatchLoaderDynamic(Instance, vkGetInstanceProcAddr);

  if (bUseDebugMessenger) {
    vk::DebugUtilsMessengerCreateInfoEXT DebugUtilsMessengerCreateInfo;
    DebugUtilsMessengerCreateInfo.setMessageSeverity(  // vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
    DebugUtilsMessengerCreateInfo.setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                                 vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                                 vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance);
    DebugUtilsMessengerCreateInfo.setPfnUserCallback(&DebugUtilsMessengerCallback);
    DebugUtilsMessengerCreateInfo.setPUserData(nullptr);

    vk::ResultValue<vk::DebugUtilsMessengerEXT> DebugUtilsMessengerResult = Instance.createDebugUtilsMessengerEXT(
        DebugUtilsMessengerCreateInfo, nullptr, DispatchLoaderDynamic);
    VK_SUCCEEDED(DebugUtilsMessengerResult.result);
    DebugUtilsMessenger = DebugUtilsMessengerResult.value;
  }
}

void CVKRenderer::CreateSurface() {
  VkSurfaceKHR SDLVulkanSurface;
  X_RUNTIME_ASSERT(SDL_Vulkan_CreateSurface(MainWindow, Instance, &SDLVulkanSurface) == SDL_bool::SDL_TRUE);
  Surface = SDLVulkanSurface;
}

void CVKRenderer::CreateDevice() {
  std::vector<int32> PhysicalDeviceScores;
  vk::ResultValue<std::vector<vk::PhysicalDevice>> AvailablePhysicalDevicesResult = Instance.enumeratePhysicalDevices();
  VK_SUCCEEDED(AvailablePhysicalDevicesResult.result);
  SPDLOG_ERROR("PhysicalDevices :");
  for (auto& AvailablePhysicalDevice : AvailablePhysicalDevicesResult.value) {
    PhysicalDeviceScores.push_back(0);
    vk::PhysicalDeviceProperties PhysicalDeviceProperties = AvailablePhysicalDevice.getProperties();
    SPDLOG_ERROR("    DeviceName : {}", PhysicalDeviceProperties.deviceName.data());
    switch (AvailablePhysicalDevice.getProperties().deviceType) {
      case vk::PhysicalDeviceType::eDiscreteGpu:
        PhysicalDeviceScores.back() = 0x40000000;
      case vk::PhysicalDeviceType::eIntegratedGpu:
        PhysicalDeviceScores.back() = 0x30000000;
      case vk::PhysicalDeviceType::eCpu:
        PhysicalDeviceScores.back() = 0x20000000;
      case vk::PhysicalDeviceType::eVirtualGpu:
        PhysicalDeviceScores.back() = 0x10000000;
      case vk::PhysicalDeviceType::eOther:
      default:
        break;
    }

    std::unordered_set<std::string> RequiredLayers = {};
    vk::ResultValue<std::vector<vk::LayerProperties>> SupportedLayers =
        AvailablePhysicalDevice.enumerateDeviceLayerProperties();
    VK_SUCCEEDED(SupportedLayers.result);
    for (auto& SupportedLayer : SupportedLayers.value) {
      RequiredLayers.erase(SupportedLayer.layerName.data());
    }
    std::unordered_set<std::string> RequiredExtensions = {"VK_KHR_swapchain"};

    vk::ResultValue<std::vector<vk::ExtensionProperties>> SupportedExtensions =
        AvailablePhysicalDevice.enumerateDeviceExtensionProperties();
    VK_SUCCEEDED(SupportedExtensions.result);
    for (auto& SupportedExtension : SupportedExtensions.value) {
      RequiredExtensions.erase(SupportedExtension.extensionName.data());
    }
    if (!RequiredLayers.empty() || !RequiredExtensions.empty()) {
      PhysicalDeviceScores.back() = -1;
      break;
    }

    bool bSupportGraphicsQueue = false;
    std::vector<vk::QueueFamilyProperties> QueueFamilyProperties = AvailablePhysicalDevice.getQueueFamilyProperties();
    for (int32 i = 0; i < QueueFamilyProperties.size(); i++) {
      if (QueueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) {
        vk::ResultValue<uint32> SurfaceSupportResult = AvailablePhysicalDevice.getSurfaceSupportKHR(i, Surface);
        if (SurfaceSupportResult.result == vk::Result::eSuccess && SurfaceSupportResult.value != 0) {
          bSupportGraphicsQueue = true;
          break;
        }
      }
    }

    if (!bSupportGraphicsQueue) {
      PhysicalDeviceScores.back() = -1;
      break;
    }
  }

  int32 MaxPhysicalDeviceScore = 0;
  int32 PhysicalDeviceIndex = -1;
  for (int32 i = 0; i < PhysicalDeviceScores.size(); i++) {
    int32& PhysicalDeviceScore = PhysicalDeviceScores[i];
    if (PhysicalDeviceScore > MaxPhysicalDeviceScore) {
      MaxPhysicalDeviceScore = PhysicalDeviceScore;
      PhysicalDeviceIndex = i;
    }
  }
  X_RUNTIME_ASSERT(PhysicalDeviceIndex >= 0, "NOT FIND SUPPORTED PHYSICAL DEVICE");
  PhysicalDevice = AvailablePhysicalDevicesResult.value[PhysicalDeviceIndex];

  SPDLOG_ERROR("Use PhysicalDevices : {}", PhysicalDevice.getProperties().deviceName.data());

  SPDLOG_ERROR("Support DeviceLayers :");
  vk::ResultValue<std::vector<vk::LayerProperties>> SupportedLayers = PhysicalDevice.enumerateDeviceLayerProperties();
  VK_SUCCEEDED(SupportedLayers.result);
  for (auto& SupportedLayer : SupportedLayers.value) {
#ifndef NDEBUG
    SPDLOG_ERROR("    Layer : {}", SupportedLayer.layerName.data());
#endif
  }

  SPDLOG_ERROR("Support DeviceExtensions :");
  vk::ResultValue<std::vector<vk::ExtensionProperties>> SupportedExtensions =
      PhysicalDevice.enumerateDeviceExtensionProperties();
  VK_SUCCEEDED(SupportedExtensions.result);
  for (auto& SupportedExtension : SupportedExtensions.value) {
#ifndef NDEBUG
    SPDLOG_ERROR("    Extension : {}", SupportedExtension.extensionName.data());
#endif
  }

  std::vector<vk::QueueFamilyProperties> QueueFamilyProperties = PhysicalDevice.getQueueFamilyProperties();
  for (uint32 i = 0; i < QueueFamilyProperties.size(); i++) {
    if (QueueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) {
      GraphicsFamilyIndex = i;
      vk::ResultValue<uint32> PresentFamilyIndexResult = PhysicalDevice.getSurfaceSupportKHR(i, Surface);
      if (PresentFamilyIndexResult.result == vk::Result::eSuccess && PresentFamilyIndexResult.value != 0) {
        PresentFamilyIndex = i;
        break;
      }
    }
  }

  float QueuePriority = 1.0f;

  vk::DeviceQueueCreateInfo DeviceQueueCreateInfo[2];
  // DeviceQueueCreateInfo.setFlags();
  DeviceQueueCreateInfo[0].setQueueFamilyIndex(GraphicsFamilyIndex);
  DeviceQueueCreateInfo[0].setQueueCount(1);
  DeviceQueueCreateInfo[0].setPQueuePriorities(&QueuePriority);

  if (GraphicsFamilyIndex != PresentFamilyIndex) {
    DeviceQueueCreateInfo[1].setQueueFamilyIndex(PresentFamilyIndex);
    DeviceQueueCreateInfo[1].setQueueCount(1);
    DeviceQueueCreateInfo[1].setPQueuePriorities(&QueuePriority);
  }

  vk::PhysicalDeviceFeatures PhysicalDeviceFeatures;

  std::vector<const char*> DeviceEnabledLayers = {
      "VK_LAYER_KHRONOS_validation",
  };
  std::vector<const char*> DeviceEnabledExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#ifdef __APPLE__
      "VK_KHR_portability_subset",
#endif
  };
#if VK_EXT_validation_features
  if (bEnableValidationLayers) {
    DeviceEnabledExtensions.push_back("VK_KHR_shader_non_semantic_info");
  }
#endif
  vk::DeviceCreateInfo DeviceCreateInfo;
  // DeviceCreateInfo.setFlags();
  DeviceCreateInfo.setQueueCreateInfoCount(GraphicsFamilyIndex != PresentFamilyIndex ? 2 : 1);
  DeviceCreateInfo.setPQueueCreateInfos(DeviceQueueCreateInfo);
  DeviceCreateInfo.setEnabledLayerCount(DeviceEnabledLayers.size());
  DeviceCreateInfo.setPpEnabledLayerNames(DeviceEnabledLayers.data());
  DeviceCreateInfo.setEnabledExtensionCount(DeviceEnabledExtensions.size());
  DeviceCreateInfo.setPpEnabledExtensionNames(DeviceEnabledExtensions.data());
  DeviceCreateInfo.setPEnabledFeatures(&PhysicalDeviceFeatures);

  vk::ResultValue<vk::Device> DeviceResult = PhysicalDevice.createDevice(DeviceCreateInfo);
  VK_SUCCEEDED(DeviceResult.result);
  Device = DeviceResult.value;
  GraphicsQueue = Device.getQueue(GraphicsFamilyIndex, 0);
  PresentQueue = GraphicsFamilyIndex != PresentFamilyIndex ? Device.getQueue(PresentFamilyIndex, 0) : GraphicsQueue;
}

void CVKRenderer::CreateSwapchain() {
  vk::ResultValue<vk::SurfaceCapabilitiesKHR> SurfaceCapabilitiesResult = PhysicalDevice.getSurfaceCapabilitiesKHR(Surface);
  VK_SUCCEEDED(SurfaceCapabilitiesResult.result);
  SurfaceCapabilities = SurfaceCapabilitiesResult.value;

  vk::ResultValue<std::vector<vk::SurfaceFormatKHR>> SurfaceFormatsResult = PhysicalDevice.getSurfaceFormatsKHR(Surface);
  VK_SUCCEEDED(SurfaceFormatsResult.result);
  SurfaceFormats = SurfaceFormatsResult.value;

  vk::ResultValue<std::vector<vk::PresentModeKHR>> PresentModesResult = PhysicalDevice.getSurfacePresentModesKHR(Surface);
  VK_SUCCEEDED(PresentModesResult.result);
  PresentModes = PresentModesResult.value;

  SwapchainExtent = ChooseSwapchainExtent();
  SwapchainSurfaceFormat = ChooseSwapchainSurfaceFormat();
  SwapchainPresentMode = ChooseSwapchainPresentMode();
  vk::SwapchainKHR OldSwapchain = Swapchain;

  MinImageCount = std::min(SurfaceCapabilities.maxImageCount, SurfaceCapabilities.minImageCount + 1);
  vk::SwapchainCreateInfoKHR SwapchainCreateInfo;
  // SwapchainCreateInfo.setFlags();
  SwapchainCreateInfo.setSurface(Surface);
  SwapchainCreateInfo.setMinImageCount(MinImageCount);
  SwapchainCreateInfo.setImageFormat(SwapchainSurfaceFormat.format);
  SwapchainCreateInfo.setImageColorSpace(SwapchainSurfaceFormat.colorSpace);
  SwapchainCreateInfo.setImageExtent(SwapchainExtent);
  SwapchainCreateInfo.setImageArrayLayers(1);
  SwapchainCreateInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);
  uint32 QueueFamilyIndices[2];
  QueueFamilyIndices[0] = GraphicsFamilyIndex;
  if (GraphicsFamilyIndex != PresentFamilyIndex) {
    QueueFamilyIndices[1] = PresentFamilyIndex;
    SwapchainCreateInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
    SwapchainCreateInfo.setQueueFamilyIndexCount(2);
  } else {
    SwapchainCreateInfo.setImageSharingMode(vk::SharingMode::eExclusive);
    SwapchainCreateInfo.setQueueFamilyIndexCount(1);
  }
  SwapchainCreateInfo.setPQueueFamilyIndices(QueueFamilyIndices);
  SwapchainCreateInfo.setPreTransform(SurfaceCapabilities.currentTransform);
  SwapchainCreateInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
  SwapchainCreateInfo.setPresentMode(SwapchainPresentMode);
  SwapchainCreateInfo.setClipped(VK_TRUE);
  SwapchainCreateInfo.setOldSwapchain(OldSwapchain);

  vk::ResultValue<vk::SwapchainKHR> SwapchainResult = Device.createSwapchainKHR(SwapchainCreateInfo);
  VK_SUCCEEDED(SwapchainResult.result);
  Swapchain = SwapchainResult.value;

  if (OldSwapchain) Device.destroySwapchainKHR(OldSwapchain);

  vk::ResultValue<std::vector<vk::Image>> SwapchainImagesResult = Device.getSwapchainImagesKHR(Swapchain);
  VK_SUCCEEDED(SwapchainImagesResult.result);
  X_ASSERT(RenderFrames.size() == 0 || RenderFrames.size() == SwapchainImagesResult.value.size());

  MaxFramesInFlight = SwapchainImagesResult.value.size();
  if (RenderFrames.empty()) {
    RenderFrames.resize(MaxFramesInFlight);
  }
  X_ASSERT(MaxFramesInFlight == RenderFrames.size());
  for (int32 i = 0; i < RenderFrames.size(); i++) {
    RenderFrames[i].SwapchainImage = SwapchainImagesResult.value[i];
    vk::ImageViewCreateInfo ImageViewCreateInfo;
    ImageViewCreateInfo.setImage(RenderFrames[i].SwapchainImage);
    ImageViewCreateInfo.setViewType(vk::ImageViewType::e2D);
    ImageViewCreateInfo.setFormat(SwapchainSurfaceFormat.format);
    ImageViewCreateInfo.components.r = vk::ComponentSwizzle::eIdentity;
    ImageViewCreateInfo.components.g = vk::ComponentSwizzle::eIdentity;
    ImageViewCreateInfo.components.b = vk::ComponentSwizzle::eIdentity;
    ImageViewCreateInfo.components.a = vk::ComponentSwizzle::eIdentity;
    ImageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    ImageViewCreateInfo.subresourceRange.levelCount = 1;
    ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    ImageViewCreateInfo.subresourceRange.layerCount = 1;
    vk::ResultValue<vk::ImageView> SwapchainImageViewResult = Device.createImageView(ImageViewCreateInfo);
    VK_SUCCEEDED(SwapchainImageViewResult.result);
    RenderFrames[i].SwapchainImageView = SwapchainImageViewResult.value;
  }
  RenderFrameIndex = 0;
}

void CVKRenderer::CreateRenderPass() {
  std::vector<vk::AttachmentReference> ColorAttachmentRefs;
  ColorAttachmentRefs.push_back({});
  ColorAttachmentRefs.back().attachment = 0;
  ColorAttachmentRefs.back().layout = vk::ImageLayout::eColorAttachmentOptimal;

  vk::SubpassDescription SubpassDescription;
  SubpassDescription.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
  SubpassDescription.inputAttachmentCount = 0;
  SubpassDescription.pInputAttachments = nullptr;
  SubpassDescription.colorAttachmentCount = ColorAttachmentRefs.size();
  SubpassDescription.pColorAttachments = ColorAttachmentRefs.data();
  SubpassDescription.pResolveAttachments = nullptr;
  SubpassDescription.pDepthStencilAttachment = nullptr;
  // SubpassDescription.preserveAttachmentCount = 0;
  // SubpassDescription.pPreserveAttachments = nullptr;

  std::vector<vk::AttachmentDescription> ColorAttachmentDescs;
  ColorAttachmentDescs.push_back({});
  ColorAttachmentDescs.back().format = SwapchainSurfaceFormat.format;
  ColorAttachmentDescs.back().samples = vk::SampleCountFlagBits::e1;
  ColorAttachmentDescs.back().loadOp = vk::AttachmentLoadOp::eClear;
  ColorAttachmentDescs.back().storeOp = vk::AttachmentStoreOp::eStore;
  ColorAttachmentDescs.back().stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
  ColorAttachmentDescs.back().stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
  ColorAttachmentDescs.back().initialLayout = vk::ImageLayout::eUndefined;
  ColorAttachmentDescs.back().finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

  vk::RenderPassCreateInfo RenderPassCreateInfo;
  RenderPassCreateInfo.attachmentCount = ColorAttachmentDescs.size();
  RenderPassCreateInfo.pAttachments = ColorAttachmentDescs.data();
  RenderPassCreateInfo.subpassCount = 1;
  RenderPassCreateInfo.pSubpasses = &SubpassDescription;
  // RenderPassCreateInfo.dependencyCount = 0;
  // RenderPassCreateInfo.pDependencies = nullptr;

  auto RenderPassResult = Device.createRenderPass(RenderPassCreateInfo);
  VK_SUCCEEDED(RenderPassResult.result);
  RenderPass = RenderPassResult.value;
}

void CVKRenderer::CreateFramebuffers() {
  for (size_t i = 0; i < RenderFrames.size(); i++) {
    std::vector<vk::ImageView> Attachments = {RenderFrames[i].SwapchainImageView};
    vk::FramebufferCreateInfo FramebufferCreateInfo;
    FramebufferCreateInfo.renderPass = RenderPass;
    FramebufferCreateInfo.attachmentCount = Attachments.size();
    FramebufferCreateInfo.pAttachments = Attachments.data();
    FramebufferCreateInfo.width = SwapchainExtent.width;
    FramebufferCreateInfo.height = SwapchainExtent.height;
    FramebufferCreateInfo.layers = 1;
    auto FramebufferResult = Device.createFramebuffer(FramebufferCreateInfo);
    VK_SUCCEEDED(FramebufferResult.result);
    RenderFrames[i].Framebuffer = FramebufferResult.value;
  }
}

void CVKRenderer::CreateSyncObjects() {
  for (size_t i = 0; i < RenderFrames.size(); i++) {
    vk::SemaphoreCreateInfo SemaphoreCreateInfo;
    auto ImageAvailableSemaphoreResult = Device.createSemaphore(SemaphoreCreateInfo);
    VK_SUCCEEDED(ImageAvailableSemaphoreResult.result);
    RenderFrames[i].ImageAvailableSemaphore = ImageAvailableSemaphoreResult.value;

    auto RenderFinishedSemaphoreResult = Device.createSemaphore(SemaphoreCreateInfo);
    VK_SUCCEEDED(RenderFinishedSemaphoreResult.result);
    RenderFrames[i].RenderFinishedSemaphore = RenderFinishedSemaphoreResult.value;

    vk::FenceCreateInfo FenceCreateInfo;
    FenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;
    auto FenceResult = Device.createFence(FenceCreateInfo);
    VK_SUCCEEDED(FenceResult.result);
    RenderFrames[i].InFlightFence = FenceResult.value;
  }
}

void CVKRenderer::CreateCommandPool() {
  vk::CommandPoolCreateInfo CommandPoolCreateInfo;
  CommandPoolCreateInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
  CommandPoolCreateInfo.queueFamilyIndex = GraphicsFamilyIndex;
  auto CommandPoolResult = Device.createCommandPool(CommandPoolCreateInfo);
  VK_SUCCEEDED(CommandPoolResult.result);
  CommandPool = CommandPoolResult.value;

}

void CVKRenderer::AllocateCommandBuffers() {
  for (size_t i = 0; i < RenderFrames.size(); i++) {
    vk::CommandBufferAllocateInfo CommandBufferAllocateInfo;
    CommandBufferAllocateInfo.commandPool = CommandPool;
    CommandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
    CommandBufferAllocateInfo.commandBufferCount = 1;
    auto CommandBuffersResult = Device.allocateCommandBuffers(CommandBufferAllocateInfo);
    VK_SUCCEEDED(CommandBuffersResult.result);
    RenderFrames[i].CommandBuffer = CommandBuffersResult.value[0];
  }
  vk::CommandBufferAllocateInfo CommandBufferAllocateInfo;
  CommandBufferAllocateInfo.commandPool = CommandPool;
  CommandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
  CommandBufferAllocateInfo.commandBufferCount = 1;

  auto CommandBuffersResult = Device.allocateCommandBuffers(CommandBufferAllocateInfo);
  VK_SUCCEEDED(CommandBuffersResult.result);
  MainCommandBuffer = CommandBuffersResult.value[0];
}

void CVKRenderer::RecreateSwapchain() {
  SPDLOG_ERROR("RecreateSwapchain");
  (void)Device.waitIdle();
  DestroyFramebuffers();
  for (size_t i = 0; i < RenderFrames.size(); i++) {
    Device.destroyImageView(RenderFrames[i].SwapchainImageView);
  }
  CreateSwapchain();
  CreateFramebuffers();
}

void CVKRenderer::DestroyCommandPool() {
  Device.destroyCommandPool(CommandPool);
}

void CVKRenderer::DestroySyncObjects() {
  for (size_t i = 0; i < RenderFrames.size(); i++) {
    Device.destroyFence(RenderFrames[i].InFlightFence);
    Device.destroySemaphore(RenderFrames[i].RenderFinishedSemaphore);
    Device.destroySemaphore(RenderFrames[i].ImageAvailableSemaphore);
  }
}

void CVKRenderer::DestroyFramebuffers() {
  for (size_t i = 0; i < RenderFrames.size(); i++) {
    Device.destroyFramebuffer(RenderFrames[i].Framebuffer);
  }
}

void CVKRenderer::DestroyRenderPass() {
  Device.destroyRenderPass(RenderPass);
}

void CVKRenderer::DestroySwapchain() {
  for (size_t i = 0; i < RenderFrames.size(); i++) {
    Device.destroyImageView(RenderFrames[i].SwapchainImageView);
  }
  Device.destroySwapchainKHR(Swapchain);
}

void CVKRenderer::DestroyDevice() {
  Device.destroy();
}

void CVKRenderer::DestroySurface() {
  Instance.destroySurfaceKHR(Surface);
}

void CVKRenderer::DestroyInstance() {
  if (bUseDebugMessenger) {
    Instance.destroyDebugUtilsMessengerEXT(DebugUtilsMessenger, nullptr, DispatchLoaderDynamic);
  }
  Instance.destroy();
}

VkBool32 CVKRenderer::DebugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                             VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                             const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                             void* pUserData) {
#if VK_EXT_validation_features
  if (strcmp(pCallbackData->pMessageIdName, "UNASSIGNED-DEBUG-PRINTF") == 0) {
    SPDLOG_ERROR("{}", pCallbackData->pMessage);
    return VK_FALSE;
  }
#endif
  std::stringstream ReportFormatter;
  ReportFormatter << vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(messageSeverity)) << ": "
                  << vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(messageType)) << ":\n";
  ReportFormatter << std::string("\t") << "messageIDName   = <" << pCallbackData->pMessageIdName << ">\n";
  ReportFormatter << std::string("\t") << "messageIdNumber = " << pCallbackData->messageIdNumber << "\n";
  ReportFormatter << std::string("\t") << "message         = <" << pCallbackData->pMessage << ">\n";
  if (0 < pCallbackData->queueLabelCount) {
    ReportFormatter << std::string("\t") << "Queue Labels:\n";
    for (uint32_t i = 0; i < pCallbackData->queueLabelCount; i++) {
      ReportFormatter << std::string("\t\t") << "labelName = <" << pCallbackData->pQueueLabels[i].pLabelName << ">\n";
    }
  }
  if (0 < pCallbackData->cmdBufLabelCount) {
    ReportFormatter << std::string("\t") << "CommandBuffer Labels:\n";
    for (uint32_t i = 0; i < pCallbackData->cmdBufLabelCount; i++) {
      ReportFormatter << std::string("\t\t") << "labelName = <" << pCallbackData->pCmdBufLabels[i].pLabelName << ">\n";
    }
  }
  if (0 < pCallbackData->objectCount) {
    ReportFormatter << std::string("\t") << "Objects:\n";
    for (uint32_t i = 0; i < pCallbackData->objectCount; i++) {
      ReportFormatter << std::string("\t\t") << "Object " << i << "\n";
      ReportFormatter << std::string("\t\t\t")
                      << "objectType   = " << vk::to_string(static_cast<vk::ObjectType>(pCallbackData->pObjects[i].objectType))
                      << "\n";
      ReportFormatter << std::string("\t\t\t") << "objectHandle = " << pCallbackData->pObjects[i].objectHandle << "\n";
      if (pCallbackData->pObjects[i].pObjectName) {
        ReportFormatter << std::string("\t\t\t") << "objectName   = <" << pCallbackData->pObjects[i].pObjectName << ">\n";
      }
    }
  }
  SPDLOG_ERROR("{}", ReportFormatter.str());
  return VK_FALSE;
}

//
//IShader* CVKRenderer::CreateShader(EShaderType shader_type, const char* source, const char* entry_point) {
//  std::string shader_code = Utils::LoadCodeFromFile(source);
//  const glslang_input_t input = {.language = GLSLANG_SOURCE_GLSL,
//                                 .stage = GetGLSLangStageShaderType(shader_type),
//                                 .client = GLSLANG_CLIENT_VULKAN,
//                                 .client_version = GLSLANG_TARGET_VULKAN_1_1,
//                                 .target_language = GLSLANG_TARGET_SPV,
//                                 .target_language_version = GLSLANG_TARGET_SPV_1_3,
//                                 .code = shader_code.c_str(),
//                                 .default_version = 460,
//                                 .default_profile = GLSLANG_CORE_PROFILE,
//                                 .force_default_version_and_profile = 0,
//                                 .forward_compatible = 0,
//                                 .messages = GLSLANG_MSG_DEFAULT_BIT,
//                                 .resource = glslang_default_resource()};
//  if (glslang_shader_t* glslang_shader = glslang_shader_create(&input)) {
//    if (glslang_shader_preprocess(glslang_shader, &input)) {
//      if (!glslang_shader_parse(glslang_shader, &input)) {
//        if (glslang_program_t* glslang_program = glslang_program_create()) {
//          glslang_program_add_shader(glslang_program, glslang_shader);
//          if (glslang_program_link(glslang_program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT)) {
//            glslang_program_SPIRV_generate(glslang_program, input.stage);
//            if (auto messages = glslang_program_SPIRV_get_messages(glslang_program)) {
//              SPDLOG_WARN("glslang_program_SPIRV_get_messages : {}", messages);
//            }
//            vk::ShaderModuleCreateInfo shader_module_create_info;
//            shader_module_create_info.codeSize = glslang_program_SPIRV_get_size(glslang_program) * sizeof(unsigned int);
//            shader_module_create_info.pCode = glslang_program_SPIRV_get_ptr(glslang_program);
//            auto ShaderModuleResult = Device.createShaderModule(shader_module_create_info);
//            VK_SUCCEEDED(ShaderModuleResult.result);
//            CVKShader* shader = new CVKShader(shader_type);
//            shader->ShaderModule = ShaderModuleResult.value;
//            return shader;
//          } else {
//            LastErrorCode = -1;
//            LastErrorMessage = fmt::format("glslang_program_link : {} ({})!", glslang_shader_get_info_log(glslang_shader),
//                                           glslang_shader_get_info_debug_log(glslang_shader));
//          }
//          glslang_program_delete(glslang_program);
//        } else {
//          LastErrorCode = -1;
//          LastErrorMessage = fmt::format("glslang_program_create : {} ({})!", glslang_shader_get_info_log(glslang_shader),
//                                         glslang_shader_get_info_debug_log(glslang_shader));
//        }
//      } else {
//        LastErrorCode = -1;
//        LastErrorMessage = fmt::format("glslang_shader_parse : {} ({})!", glslang_shader_get_info_log(glslang_shader),
//                                       glslang_shader_get_info_debug_log(glslang_shader));
//      }
//    } else {
//      LastErrorCode = -1;
//      LastErrorMessage = fmt::format("glslang_shader_preprocess : {} ({})!", glslang_shader_get_info_log(glslang_shader),
//                                     glslang_shader_get_info_debug_log(glslang_shader));
//    }
//    glslang_shader_delete(glslang_shader);
//  } else {
//    LastErrorCode = -1;
//    LastErrorMessage = fmt::format("glslang_shader_create : {} ({})!", glslang_shader_get_info_log(glslang_shader),
//                                   glslang_shader_get_info_debug_log(glslang_shader));
//  }
//  return nullptr;
//}
//
//IBuffer* CVKRenderer::CreateBuffer(void* init_data, uint64 buffer_size) {
//  return nullptr;
//}

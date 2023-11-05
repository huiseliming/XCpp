#include "VKImGuiLayer.h"
#include "Renderer/Vulkan/VKRenderer.h"
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_vulkan.h>

void CVKImGuiLayer::Init(SDL_Window* main_window, IRenderer* renderer) {
  MainWindow = main_window;
  Renderer = renderer;

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& IO = ImGui::GetIO();
  IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  IO.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  IO.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
  IO.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;
  IO.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;

  ImGui::StyleColorsDark();

  CreateRenderPass();
  CreateFramebuffers();
  AllocateCommandBuffers();
  // Create Descriptor Pool
  {
    VkDescriptorPoolSize DescriptorPoolSize[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                                                 {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                                                 {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                                                 {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                                                 {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                                                 {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                                                 {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                                                 {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                                                 {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                                                 {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                                                 {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};
    VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo = {};
    DescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    DescriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    DescriptorPoolCreateInfo.maxSets = 1000 * IM_ARRAYSIZE(DescriptorPoolSize);
    DescriptorPoolCreateInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(DescriptorPoolSize);
    DescriptorPoolCreateInfo.pPoolSizes = DescriptorPoolSize;
    auto ImGuiDescriptorPoolResult = GetVKRenderer()->Device.createDescriptorPool(DescriptorPoolCreateInfo);
    VK_SUCCEEDED(ImGuiDescriptorPoolResult.result);
    DescriptorPool = ImGuiDescriptorPoolResult.value;
  }

  ImGui_ImplSDL2_InitForVulkan(MainWindow);
  ImGui_ImplVulkan_InitInfo InitInfo = {};
  InitInfo.Instance = GetVKRenderer()->Instance;
  InitInfo.PhysicalDevice = GetVKRenderer()->PhysicalDevice;
  InitInfo.Device = GetVKRenderer()->Device;
  InitInfo.QueueFamily = GetVKRenderer()->GraphicsFamilyIndex;
  InitInfo.Queue = GetVKRenderer()->GraphicsQueue;
  InitInfo.PipelineCache = VK_NULL_HANDLE;
  InitInfo.DescriptorPool = DescriptorPool;
  InitInfo.Subpass = 0;
  InitInfo.MinImageCount = GetVKRenderer()->MinImageCount;
  InitInfo.ImageCount = GetVKRenderer()->MaxFramesInFlight;
  InitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  InitInfo.Allocator = nullptr;
  InitInfo.CheckVkResultFn = [](VkResult) {};
  ImGui_ImplVulkan_Init(&InitInfo, RenderPass);

  {
    auto command_buffer = GetVKRenderer()->AllocateAndBeginCommandBuffer();
    ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
    GetVKRenderer()->SubmitCommandBufferWaitIdle(command_buffer);
  }

  ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void CVKImGuiLayer::NewFrame() {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();
}

void CVKImGuiLayer::DrawFrame() {
  CImGuiLayer::DrawFrame();
}

void CVKImGuiLayer::RenderFrame() {
  ImGuiIO& io = ImGui::GetIO();
  auto& RenderFrame = RenderFrames[GetVKRenderer()->RenderFrameIndex];

  RenderFrame.CommandBuffer.reset();
  vk::CommandBufferBeginInfo CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
  VK_SUCCEEDED(RenderFrame.CommandBuffer.begin(CommandBufferBeginInfo));

  vk::ClearValue clear_color = vk::ClearColorValue{ 0.f, 0.f, 0.f, 1.f };
  vk::RenderPassBeginInfo RenderPassBeginInfo;
  RenderPassBeginInfo.renderPass = RenderPass;
  RenderPassBeginInfo.framebuffer = RenderFrame.Framebuffer;
  RenderPassBeginInfo.renderArea = vk::Rect2D(vk::Offset2D(), GetVKRenderer()->SwapchainExtent);
  RenderPassBeginInfo.clearValueCount = 1;
  RenderPassBeginInfo.pClearValues = &clear_color;

  RenderFrame.CommandBuffer.beginRenderPass(RenderPassBeginInfo, vk::SubpassContents::eInline);
  ImGui::Render();
  ImDrawData* DrawData = ImGui::GetDrawData();
  const bool bIsMinimized = (DrawData->DisplaySize.x <= 0.0f || DrawData->DisplaySize.y <= 0.0f);
  if (!bIsMinimized) {
    ImGui_ImplVulkan_RenderDrawData(DrawData, RenderFrame.CommandBuffer);
  }

  RenderFrame.CommandBuffer.endRenderPass();
  VK_SUCCEEDED(RenderFrame.CommandBuffer.end());
  if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
  }

}

void CVKImGuiLayer::Shutdown() {
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  GetVKRenderer()->Device.destroyDescriptorPool(DescriptorPool);
  DestroyFramebuffers();
  DestroyRenderPass();
  ImGui::DestroyContext();
}

void CVKImGuiLayer::CreateRenderPass() {
  vk::AttachmentDescription Attachments[] = {vk::AttachmentDescription(
      vk::AttachmentDescriptionFlags(), GetVKRenderer()->SwapchainSurfaceFormat.format, vk::SampleCountFlagBits::e1,
      vk::AttachmentLoadOp::eLoad, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare,
      vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR)};

  vk::AttachmentReference AttachmentRefs[] = {vk::AttachmentReference{0, vk::ImageLayout::eColorAttachmentOptimal}};

  vk::SubpassDescription Subpasses[] = {vk::SubpassDescription(vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics,
                                                               nullptr, AttachmentRefs, nullptr, nullptr, nullptr)};

  vk::SubpassDependency Dependencies[] = {vk::SubpassDependency{
      VK_SUBPASS_EXTERNAL,
      0,
      vk::PipelineStageFlagBits::eColorAttachmentOutput,
      vk::PipelineStageFlagBits::eColorAttachmentOutput,
      vk::AccessFlagBits::eNoneKHR,
      vk::AccessFlagBits::eColorAttachmentWrite,
      vk::DependencyFlagBits(),
  }};

  vk::RenderPassCreateInfo RenderPassCreateInfo(vk::RenderPassCreateFlags(), Attachments, Subpasses, Dependencies);
  auto ImGuiRenderPassResult = GetVKRenderer()->Device.createRenderPass(RenderPassCreateInfo);
  VK_SUCCEEDED(ImGuiRenderPassResult.result);
  RenderPass = ImGuiRenderPassResult.value;
}

void CVKImGuiLayer::CreateFramebuffers() {
  RenderFrames.resize(GetVKRenderer()->MaxFramesInFlight);
  for (size_t i = 0; i < GetVKRenderer()->MaxFramesInFlight; i++) {
    vk::ImageView Attachments[] = {GetVKRenderer()->RenderFrames[i].SwapchainImageView};
    vk::FramebufferCreateInfo FramebufferCreateInfo;
    FramebufferCreateInfo.renderPass = RenderPass;
    FramebufferCreateInfo.attachmentCount = std::size(Attachments);
    FramebufferCreateInfo.pAttachments = Attachments;
    FramebufferCreateInfo.width = GetVKRenderer()->SwapchainExtent.width;
    FramebufferCreateInfo.height = GetVKRenderer()->SwapchainExtent.height;
    FramebufferCreateInfo.layers = 1;
    auto ImGuiFramebufferResult = GetVKRenderer()->Device.createFramebuffer(FramebufferCreateInfo);
    VK_SUCCEEDED(ImGuiFramebufferResult.result);
    RenderFrames[i].Framebuffer = ImGuiFramebufferResult.value;
  }
}

void CVKImGuiLayer::AllocateCommandBuffers() {
  for (size_t i = 0; i < GetVKRenderer()->MaxFramesInFlight; i++) {
    vk::CommandBufferAllocateInfo CommandBufferAllocateInfo;
    CommandBufferAllocateInfo.commandPool = GetVKRenderer()->CommandPool;
    CommandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
    CommandBufferAllocateInfo.commandBufferCount = 1;

    auto CommandBuffersResult = GetVKRenderer()->Device.allocateCommandBuffers(CommandBufferAllocateInfo);
    VK_SUCCEEDED(CommandBuffersResult.result);
    RenderFrames[i].CommandBuffer = CommandBuffersResult.value[0];
  }
}

void CVKImGuiLayer::DestroyFramebuffers() {
  for (size_t i = 0; i < RenderFrames.size(); i++) {
    GetVKRenderer()->Device.destroyFramebuffer(RenderFrames[i].Framebuffer);
  }
}

void CVKImGuiLayer::DestroyRenderPass() {
  GetVKRenderer()->Device.destroyRenderPass(RenderPass);
}

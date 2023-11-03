#pragma once
#include "../ImGuiLayer.h"
#include "VKUtils.h"

struct SImGuiLayerRenderFrame {
  vk::Framebuffer Framebuffer;
  vk::CommandBuffer CommandBuffer;
};


class XRENDERER_API CVKImGuiLayer : public CImGuiLayer {
 public:
  virtual void Init(SDL_Window* main_window, IRenderer* renderer) override;
  virtual void NewFrame() override;
  virtual void DrawFrame() override;
  virtual void RenderFrame() override;
  virtual void Shutdown() override;


  void CreateRenderPass();
  void CreateFramebuffers();
  void AllocateCommandBuffers();
  void DestroyFramebuffers();
  void DestroyRenderPass();

  vk::RenderPass GetRenderPass() { return RenderPass; }
  CVKRenderer* GetVKRenderer() { return reinterpret_cast<CVKRenderer*>(Renderer); }

 protected:
  vk::DescriptorPool DescriptorPool;
  vk::RenderPass RenderPass;
  std::vector<SImGuiLayerRenderFrame> RenderFrames;

 private:
  friend class CVKRenderer;
};

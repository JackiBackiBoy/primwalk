#ifndef PW_RENDERER_HEADER
#define PW_RENDERER_HEADER

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/color.hpp"
#include "primwalk/window.hpp"
#include "primwalk/rendering/framebuffer.hpp"
#include "primwalk/rendering/image.hpp"
#include "primwalk/rendering/renderpass.hpp"
#include "primwalk/ui/subView.hpp"

// std
#include <atomic>
#include <memory>
#include <vector>

namespace pw {
  class PW_API Renderer {
    public:
      Renderer(Window& window);
      ~Renderer();

      VkCommandBuffer beginFrame();
      bool endFrame();
      void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
      void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
      std::atomic<bool> m_FramebufferResized = false;

      void submitSubView(std::unique_ptr<SubView> subView);

      // Getters
      int getFrameIndex() const;
      size_t getCurrentFrame() const;
      VkRenderPass getSwapChainRenderPass() const;
      uint32_t getSwapChainWidth() const;
      uint32_t getSwapChainHeight() const;
      static VkSampler m_TextureSampler;
      std::vector<std::unique_ptr<SubView>> m_SubViews;

      // Setters
      inline void setClearColor(Color color) { m_ClearColor = color; }

    private:
      void recreateSwapChain();
      void cleanupSwapChain();
      void createSwapChain();
      void createImages();
      void createTextureSampler();
      void createRenderPass();
      void createFramebuffers();
      void createOffscreen();
      void createSyncObjects();
      void createCommandBuffers();

      VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);
      VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
      VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
      VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

      Window& m_Window;

      VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
      VkExtent2D m_Extent;
      VkSurfaceFormatKHR m_BestSurfaceFormat;
      std::unique_ptr<RenderPass> m_RenderPass;
      

      std::vector<std::unique_ptr<Image>> m_SwapChainImages;
      std::vector<std::unique_ptr<Framebuffer>> m_SwapChainFramebuffers;
      std::vector<VkSemaphore> m_ImageAvailableSemaphores;
      std::vector<VkSemaphore> m_RenderFinishedSemaphores;
      std::vector<VkFence> m_InFlightFences;
      std::vector<VkFence> m_ImagesInFlight;
      std::vector<VkCommandBuffer> m_CommandBuffers;
      Color m_ClearColor = { 0, 0, 0 };
      uint32_t m_CurrentImageIndex = 0;
      size_t m_CurrentFrame = 0;
      int m_CurrentFrameIndex{};
  };
}

#endif
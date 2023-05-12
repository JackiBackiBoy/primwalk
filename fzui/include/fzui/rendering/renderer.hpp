#ifndef FZ_RENDERER_HEADER
#define FZ_RENDERER_HEADER

// FZUI
#include "fzui/core.hpp"
#include "fzui/window.hpp"
#include "fzui/rendering/graphicsDevice.hpp"

// std
#include <vector>
#include <atomic>

namespace fz {
  class FZ_API Renderer {
    public:
      static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

      Renderer(Window& window, GraphicsDevice_Vulkan& device);
      ~Renderer();

      VkCommandBuffer beginFrame();
      bool endFrame();
      void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
      void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
      std::atomic<bool> m_FramebufferResized = false;

      // Getters
      int getFrameIndex() const;
      size_t getCurrentFrame() const;
      VkRenderPass getSwapChainRenderPass() const;
      uint32_t getSwapChainWidth() const;
      uint32_t getSwapChainHeight() const;

    private:
      void recreateSwapChain();
      void cleanupSwapChain();
      void createSwapChain();
      void createImageViews();
      void createRenderPass();
      void createFramebuffers();
      void createSyncObjects();
      void createCommandBuffers();

      VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

      VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
      VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
      VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

      Window& m_Window;
      GraphicsDevice_Vulkan& m_Device;

      VkFormat m_SwapChainImageFormat;
      VkExtent2D m_SwapChainExtent;
      VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
      VkRenderPass m_RenderPass = VK_NULL_HANDLE;
      std::vector<VkImage> m_SwapChainImages;
      std::vector<VkImageView> m_SwapChainImageViews;
      std::vector<VkFramebuffer> m_SwapChainFramebuffers;
      std::vector<VkSemaphore> m_ImageAvailableSemaphores;
      std::vector<VkSemaphore> m_RenderFinishedSemaphores;
      std::vector<VkFence> m_InFlightFences;
      std::vector<VkFence> m_ImagesInFlight;
      std::vector<VkCommandBuffer> m_CommandBuffers;
      

      uint32_t m_CurrentImageIndex = 0;
      size_t m_CurrentFrame = 0;
      int m_CurrentFrameIndex{};
  };
}

#endif
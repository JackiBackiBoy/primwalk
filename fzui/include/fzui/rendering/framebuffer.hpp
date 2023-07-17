#ifndef FZ_FRAMEBUFFER_HEADER
#define FZ_FRAMEBUFFER_HEADER

// FZUI
#include "fzui/core.hpp"
#include "fzui/rendering/image.hpp"

// std
#include <cstdint>
#include <memory>
#include <vector>

// vendor
#include <vulkan/vulkan.h>

namespace fz {
  struct FZ_API FramebufferInfo {
    uint32_t width = 0;
    uint32_t height = 0;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    const std::vector<std::reference_wrapper<std::unique_ptr<Image>>>& attachments;
  };

  class FZ_API Framebuffer {
  public:
    Framebuffer(const FramebufferInfo& createInfo);
    ~Framebuffer() = default;

    /* Getters */
    [[nodiscard]] inline uint32_t getWidth() const { return m_Width; }
    [[nodiscard]] inline uint32_t getHeight() const { return m_Height; }
    [[nodiscard]] inline VkFramebuffer getRawFramebuffer() const { return m_FrameBuffer; }

    void destroy();
    
  private:
    VkFramebuffer m_FrameBuffer = VK_NULL_HANDLE;
    uint32_t m_Width, m_Height;
  };
}
#endif
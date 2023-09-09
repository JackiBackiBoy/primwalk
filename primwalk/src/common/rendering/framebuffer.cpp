#include "primwalk/rendering/framebuffer.hpp"
#include "primwalk/rendering/graphicsDevice.hpp"

// std
#include <stdexcept>

namespace pw {

  Framebuffer::Framebuffer(const FramebufferInfo& createInfo) :
    m_Width(createInfo.width), m_Height(createInfo.height)
  {
    GraphicsDevice_Vulkan* device = pw::GetDevice();

    // Image views for all attachments
    std::vector<VkImageView> imageViews(createInfo.attachments.size());
    for (size_t i = 0; i < imageViews.size(); i++) {
      imageViews[i] = createInfo.attachments[i].get()->getVulkanImageView();
    }

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = createInfo.renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(createInfo.attachments.size());
    framebufferInfo.pAttachments = imageViews.data();
    framebufferInfo.width = createInfo.width;
    framebufferInfo.height = createInfo.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(device->getDevice(), &framebufferInfo, nullptr, &m_FrameBuffer) != VK_SUCCESS) {
      throw std::runtime_error("VULKAN ERROR: Failed to create framebuffer!");
    }
  }

  void Framebuffer::destroy()
  {
    GraphicsDevice_Vulkan* device = pw::GetDevice();
    vkDestroyFramebuffer(device->getDevice(), m_FrameBuffer, nullptr);
  }

}

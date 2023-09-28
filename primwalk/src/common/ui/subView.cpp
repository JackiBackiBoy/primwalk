#include "primwalk/ui/subView.hpp"
#include "primwalk/rendering/graphicsDevice.hpp"

// std
#include <array>

namespace pw {

  SubView::SubView(int width, int height, glm::vec2 position) :
    m_Width(width), m_Height(height), m_Position(position)
  {
    createImages();
    createRenderPass();
    createFramebuffer();
  }

  SubView::~SubView()
  {
    m_OffscreenImage->destroy();
    m_OffscreenFramebuffer->destroy();
    vkDestroyRenderPass(pw::GetDevice()->getDevice(), m_OffscreenPass->getVulkanRenderPass(), nullptr);
  }

  void SubView::beginPass(VkCommandBuffer commandBuffer)
  {
    VkExtent2D extent{};
    extent.width = m_Width;
    extent.height = m_Height;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
    clearValues[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_OffscreenPass->getVulkanRenderPass();
    renderPassInfo.framebuffer = m_OffscreenFramebuffer->getRawFramebuffer();
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = extent;
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{ { 0, 0 }, extent };
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
  }

  void SubView::onUpdate()
  {

  }

  void SubView::onRender(const FrameInfo& frameInfo)
  {

  }

  void SubView::endPass(VkCommandBuffer commandBuffer)
  {
    vkCmdEndRenderPass(commandBuffer);
  }

  void SubView::resize(int width, int height)
  {
    m_Width = width;
    m_Height = height;

    // Recreate framebuffer and images
    vkDeviceWaitIdle(pw::GetDevice()->getDevice());

    m_OffscreenFramebuffer->destroy();
    m_OffscreenImage->destroy();

    createImages();
    createFramebuffer();
  }

  void SubView::createImages()
  {
    ImageInfo imageInfo{};
    imageInfo.width = m_Width;
    imageInfo.height = m_Height;
    imageInfo.depth = 1;
    imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.format = VK_FORMAT_B8G8R8A8_UNORM;

    m_OffscreenImage = std::make_unique<Image>(imageInfo);
  }

  void SubView::createRenderPass()
  {
    RenderPassAttachment colorAttachment = {
      m_OffscreenImage,
      VK_ATTACHMENT_LOAD_OP_CLEAR,
      VK_ATTACHMENT_STORE_OP_STORE,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    // TODO: Add depth attachment

    SubpassInfo subpass{};
    subpass.renderTargets = { 0 };

    std::vector<RenderPassAttachment> attachments = { colorAttachment };
    RenderPassInfo passInfo = {
      attachments,
      { subpass }
    };

    m_OffscreenPass = std::make_unique<RenderPass>(passInfo);
  }

  void SubView::createFramebuffer()
  {
    FramebufferInfo framebufferInfo = {
      static_cast<uint32_t>(m_Width),
      static_cast<uint32_t>(m_Height),
      m_OffscreenPass->getVulkanRenderPass(),
      { { m_OffscreenImage } }
    };

    m_OffscreenFramebuffer = std::make_unique<Framebuffer>(framebufferInfo);
  }

}

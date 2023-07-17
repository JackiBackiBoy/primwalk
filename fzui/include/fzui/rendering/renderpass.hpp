#ifndef FZ_RENDER_PASS_HEADER
#define FZ_RENDER_PASS_HEADER

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
  struct FZ_API RenderPassAttachment {
    std::unique_ptr<Image>& imageAttachment;
    VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    VkImageLayout finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  };

  struct FZ_API SubpassInfo {
    std::vector<uint32_t> renderTargets{};
    std::vector<uint32_t> subpassInputs{};
  };

  struct FZ_API RenderPassInfo {
    const std::vector<RenderPassAttachment>& attachments;
    const std::vector<SubpassInfo>& subpassInfos;
  };

  class FZ_API RenderPass {
  public:
    RenderPass(const RenderPassInfo& createInfo);

    inline VkRenderPass getVulkanRenderPass() const { return m_RenderPass; }

  private:
    VkRenderPass m_RenderPass = VK_NULL_HANDLE;
    std::vector<VkClearValue> m_ClearValues;

    struct SubpassDescription {
      VkAttachmentReference depthReference{};
      std::vector<VkAttachmentReference> colorReferences{};
      std::vector<VkAttachmentReference> resolveReferences{};
      std::vector<VkAttachmentReference> inputReferences{};
      VkSubpassDescription data{};
    };

    uint32_t m_ColorAttachmentCount = 0;
    bool m_HasDepthAttachment = false;
  };
}
#endif
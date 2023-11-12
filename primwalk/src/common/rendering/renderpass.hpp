#pragma once

// primwalk
#include "image.hpp"
#include "framebuffer.hpp"
#include "../color.hpp"

// std
#include <cstdint>
#include <memory>
#include <vector>

// vendor
#include <vulkan/vulkan.h>

namespace pw {
	struct RenderPassAttachment {
		std::unique_ptr<Image>& imageAttachment;
		VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		VkImageLayout finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	};

	struct SubpassInfo {
		std::vector<uint32_t> renderTargets{};
		std::vector<uint32_t> subpassInputs{};
	};

	struct RenderPassInfo {
		const std::vector<RenderPassAttachment>& attachments;
		const std::vector<SubpassInfo>& subpassInfos;
	};

	class RenderPass {
	public:
		RenderPass(const RenderPassInfo& createInfo);
		~RenderPass();

		void begin(Framebuffer& frameBuffer, VkCommandBuffer commandBuffer);
		void end(VkCommandBuffer commandBuffer);

		inline VkRenderPass getVulkanRenderPass() const { return m_RenderPass; }
		
		void setClearColor(uint32_t attachmentIndex, Color color);

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


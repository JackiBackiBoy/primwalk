#include "renderpass.hpp"
#include "graphicsDevice_Vulkan.hpp"
#include "../color.hpp"

// std
#include <array>
#include <stdexcept>

namespace pw {

	// TODO: This method assumes the depth attachment is passed last, fix this
	RenderPass::RenderPass(const RenderPassInfo& createInfo) {
		GraphicsDevice_Vulkan* device = (GraphicsDevice_Vulkan*&)pw::GetDevice();

		auto& attachments = createInfo.attachments;
		auto& subpassInfos = createInfo.subpassInfos;

		std::vector<VkAttachmentDescription> attachmentDescriptions(attachments.size());
		attachmentDescriptions.resize(attachments.size());
		m_ClearValues.resize(attachments.size());

		// Attachment description
		for (size_t i = 0; i < attachments.size(); i++) {
			attachmentDescriptions[i].format = attachments[i].imageAttachment->getFormat();
			attachmentDescriptions[i].samples = attachments[i].imageAttachment->getSampling();
			attachmentDescriptions[i].loadOp = attachments[i].loadOp;
			attachmentDescriptions[i].storeOp = attachments[i].storeOp;
			attachmentDescriptions[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDescriptions[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentDescriptions[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			// TODO: Add check for depth
			attachmentDescriptions[i].finalLayout = attachments[i].finalLayout;

			if (attachmentDescriptions[i].finalLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
				m_ClearValues[i].depthStencil = { 1.0f, 0 };
			}
			else {
				m_ClearValues[i].color = { 0.0f, 0.0f, 0.0f, 1.0f };
			}
		}


		// Sub-passes
		bool hasDepthAttachment = false;

		for (const auto& a : attachments) {
			if (a.finalLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
				hasDepthAttachment = true;
				break;
			}
		}

		size_t depthAttachmentIndex = 0;
		std::vector<VkAttachmentReference> colorAttachmentsRefs(hasDepthAttachment ? attachments.size() - 1 : attachments.size());

		for (size_t i = 0; i < colorAttachmentsRefs.size(); i++) {
			colorAttachmentsRefs[i].attachment = i;
			colorAttachmentsRefs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = colorAttachmentsRefs.size();
		subpass.pColorAttachments = colorAttachmentsRefs.data();

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = colorAttachmentsRefs.size(); // makes sure the attachment ID is greater than all color attachments
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		if (hasDepthAttachment) {


			subpass.pDepthStencilAttachment = &depthAttachmentRef;
		}

		// Dependencies
		std::vector<VkSubpassDependency> dependencies;
		if (subpassInfos.size() != 1)
		{
			dependencies.resize(subpassInfos.size() + 1);

			VkSubpassDependency& firstDependency = dependencies[0];
			firstDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			firstDependency.dstSubpass = 0;
			firstDependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			firstDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			firstDependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			firstDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			firstDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			firstDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			for (size_t i = 1; i < (dependencies.size() - 1); i++)
			{
				dependencies[i].srcSubpass = i - 1;
				dependencies[i].dstSubpass = i;
				dependencies[i].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dependencies[i].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				dependencies[i].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				dependencies[i].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				dependencies[i].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			}

			VkSubpassDependency& lastDependency = *(dependencies.end() - 1);
			lastDependency.srcSubpass = subpassInfos.size() - 1;
			lastDependency.dstSubpass = VK_SUBPASS_EXTERNAL;
			lastDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			lastDependency.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			lastDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			lastDependency.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			lastDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		}
		else
		{
			dependencies.resize(1);
			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependencies[0].srcAccessMask = 0;
			dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		}

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
		renderPassInfo.pAttachments = attachmentDescriptions.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();

		if (vkCreateRenderPass(device->getDevice(), &renderPassInfo, nullptr, &m_RenderPass)) {
			throw std::runtime_error("VULKAN ERROR: Failed to create render pass!");
		}
	}

	RenderPass::~RenderPass() {
		GraphicsDevice_Vulkan* device = (GraphicsDevice_Vulkan*&)pw::GetDevice();

		vkDestroyRenderPass(device->getDevice(), m_RenderPass, nullptr);
	}

	void RenderPass::begin(Framebuffer& frameBuffer, VkCommandBuffer commandBuffer, const Viewport& viewport) {
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_RenderPass;
		renderPassInfo.framebuffer = frameBuffer.getVkFramebuffer();
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent.width = frameBuffer.getWidth();
		renderPassInfo.renderArea.extent.height = frameBuffer.getHeight();
		renderPassInfo.clearValueCount = static_cast<uint32_t>(m_ClearValues.size());
		renderPassInfo.pClearValues = m_ClearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport vkViewport{};
		vkViewport.x = viewport.offsetX;
		vkViewport.y = viewport.offsetY;
		vkViewport.width = viewport.width;
		vkViewport.height = viewport.height;
		vkViewport.minDepth = viewport.minDepth;
		vkViewport.maxDepth = viewport.maxDepth;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = { frameBuffer.getWidth(), frameBuffer.getHeight() };

		vkCmdSetViewport(commandBuffer, 0, 1, &vkViewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void RenderPass::end(VkCommandBuffer commandBuffer) {
		vkCmdEndRenderPass(commandBuffer);
	}

	void RenderPass::setClearColor(uint32_t attachmentIndex, Color color) {
		assert(attachmentIndex < m_ClearValues.size());

		glm::vec4 normColor = Color::normalize(color);
		m_ClearValues[attachmentIndex].color = { normColor.r, normColor.g, normColor.b, 1.0f };
	}

}
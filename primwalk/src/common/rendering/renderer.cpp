#include "renderer.hpp"

// std
#include <algorithm>
#include <array>
#include <iostream>

namespace pw {

	Renderer::Renderer(GraphicsDevice_Vulkan& device, Window& window) :
		m_Device(device), m_Window(window) {

		createSwapChain();
		createImages();
		createRenderPass();
		createFramebuffers();
		createTextureSampler();
		createSyncObjects();
		createCommandBuffers();
	}

	Renderer::~Renderer() {
		// TODO: Check order of deletion of sampler
		vkDestroySampler(m_Device.getDevice(), m_TextureSampler, nullptr);

		// Swap chain cleanup
		for (size_t i = 0; i < m_SwapChainImages.size(); i++) {
			m_SwapChainImages[i]->destroy();
		}
		m_SwapChainImages.clear();

		if (m_SwapChain != nullptr) {
			vkDestroySwapchainKHR(m_Device.getDevice(), m_SwapChain, nullptr);
			m_SwapChain = nullptr;
		}

		for (auto& framebuffer : m_SwapChainFramebuffers) {
			framebuffer->destroy();
		}

		vkDestroyRenderPass(m_Device.getDevice(), m_RenderPass->getVulkanRenderPass(), nullptr);

		// Cleanup synchronization objects
		for (size_t i = 0; i < GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(m_Device.getDevice(), m_RenderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(m_Device.getDevice(), m_ImageAvailableSemaphores[i], nullptr);
			vkDestroyFence(m_Device.getDevice(), m_InFlightFences[i], nullptr);
		}

		// Renderer cleanup
		vkFreeCommandBuffers(
			m_Device.getDevice(),
			m_Device.getCommandPool(),
			static_cast<uint32_t>(m_CommandBuffers.size()),
			m_CommandBuffers.data());
		m_CommandBuffers.clear();
	}

	VkCommandBuffer Renderer::beginFrame() {
		vkWaitForFences(m_Device.getDevice(), 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

		// Acquire image from the swap chain
		VkResult result = vkAcquireNextImageKHR(m_Device.getDevice(), m_SwapChain, UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &m_CurrentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			return nullptr;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("VULKAN ERROR: Failed to acquire swap chain image!");
		}

		auto commandBuffer = m_CommandBuffers[m_CurrentFrameIndex];

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("VULKAN ERROR: Failed to begin recording command buffer!");
		}

		return commandBuffer;
	}

	bool Renderer::endFrame() {
		bool ret = false;

		auto commandBuffer = m_CommandBuffers[m_CurrentFrameIndex];
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("VULKAN ERROR: Failed to record command buffer!");
		}

		auto result = submitCommandBuffers(&commandBuffer, &m_CurrentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			//recreateSwapChain();
			ret = true;
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("VULKAN ERROR: Failed to present swap chain image!");
		}

		m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT;
		return ret;
	}

	void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		VkExtent2D extent{};
		extent.width = m_SwapChainFramebuffers[m_CurrentImageIndex]->getWidth();
		extent.height = m_SwapChainFramebuffers[m_CurrentImageIndex]->getHeight();

		std::array<VkClearValue, 2> clearValues{};
		glm::vec4 normColor = Color::normalize({ 29, 29, 29 }); // TODO: Make dynamic
		clearValues[0].color = { normColor.r, normColor.g, normColor.b, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_RenderPass->getVulkanRenderPass();
		renderPassInfo.framebuffer = m_SwapChainFramebuffers[m_CurrentImageIndex]->getRawFramebuffer();
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

	void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		vkCmdEndRenderPass(commandBuffer);
	}

	void Renderer::resizeSwapChain(uint32_t width, uint32_t height) {
		//vkDeviceWaitIdle(m_Device.getDevice());
		recreateSwapChain();
	}

	void Renderer::recreateSwapChain() {
		#ifdef PW_WIN32
		int width = 0, height = 0;
		RECT rc;

		// TODO: Eliminate nasty loop
		do {
			GetClientRect(((WindowWin32&)m_Window).getHandle(), &rc);

			width = rc.right - rc.left;
			height = rc.bottom - rc.top;
		} while (width == 0 || height == 0);
		#endif

		m_Device.waitForGPU();
		cleanupSwapChain();
		createSwapChain();
		createImages();
		createFramebuffers();
	}

	VkSampler Renderer::m_TextureSampler = VK_NULL_HANDLE;

	void Renderer::cleanupSwapChain() {
		for (auto& framebuffer : m_SwapChainFramebuffers) {
			framebuffer->destroy();
		}

		for (size_t i = 0; i < m_SwapChainImages.size(); i++) {
			m_SwapChainImages[i]->destroy();
		}

		vkDestroySwapchainKHR(m_Device.getDevice(), m_SwapChain, nullptr);
	}

	void Renderer::createSwapChain() {
		SwapChainSupportDetails swapChainSupport = m_Device.getSwapChainSupport();

		m_BestSurfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		m_Extent = chooseSwapExtent(swapChainSupport.capabilities);

		// Decide swap chain image count
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		// Creation info
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_Device.getSurface();
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = m_BestSurfaceFormat.format;
		createInfo.imageColorSpace = m_BestSurfaceFormat.colorSpace;
		createInfo.imageExtent = m_Extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // optional
		createInfo.pQueueFamilyIndices = nullptr; // optional
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		// Swap chain handling specification across queue families
		QueueFamilyIndices indices = m_Device.findPhysicalQueueFamilies();
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}

		if (vkCreateSwapchainKHR(m_Device.getDevice(), &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS) {
			throw std::runtime_error("VULKAN ERROR: Failed to create swap chain!");
		}
	}

	void Renderer::createImages() {
		uint32_t imageCount;

		// Retrieve swap chain images
		vkGetSwapchainImagesKHR(m_Device.getDevice(), m_SwapChain, &imageCount, nullptr);
		std::vector<VkImage> vulkanSwapchainImages(imageCount);
		vkGetSwapchainImagesKHR(m_Device.getDevice(), m_SwapChain, &imageCount, vulkanSwapchainImages.data());
		m_SwapChainImages.resize(imageCount);

		for (size_t i = 0; i < imageCount; i++) {
			SwapchainImageInfo swapchainInfo{};
			swapchainInfo.image = vulkanSwapchainImages[i];
			swapchainInfo.format = m_BestSurfaceFormat.format;
			swapchainInfo.width = m_Extent.width;
			swapchainInfo.height = m_Extent.height;
			swapchainInfo.depth = 1;

			m_SwapChainImages[i] = Image::create(swapchainInfo);
		}
	}

	void Renderer::createTextureSampler() {
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(m_Device.getPhysicalDevice(), &properties);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		if (vkCreateSampler(m_Device.getDevice(), &samplerInfo, nullptr, &m_TextureSampler) != VK_SUCCESS) {
			throw std::runtime_error("VULKAN ERROR: Failed to create texture sampler!");
		}
	}

	void Renderer::createRenderPass() {
		RenderPassAttachment attachment = {
			m_SwapChainImages[0],
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		};

		SubpassInfo subpass{};
		subpass.renderTargets = { 0 };

		std::vector<RenderPassAttachment> attachments = { attachment };
		RenderPassInfo renderPassInfo = {
			attachments,
			{ subpass }
		};

		m_RenderPass = std::make_unique<RenderPass>(renderPassInfo);
	}

	void Renderer::createFramebuffers() {
		m_SwapChainFramebuffers.resize(m_SwapChainImages.size());

		// Create framebuffers for image views
		for (size_t i = 0; i < m_SwapChainImages.size(); i++) {
			FramebufferInfo framebufferInfo = {
			m_Extent.width,
			m_Extent.height,
			m_RenderPass->getVulkanRenderPass(),
			{ { m_SwapChainImages[i] } }
			};
			m_SwapChainFramebuffers[i] = std::make_unique<Framebuffer>(framebufferInfo);
		}
	}

	void Renderer::createSyncObjects() {
		m_ImageAvailableSemaphores.resize(GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT);
		m_RenderFinishedSemaphores.resize(GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT);
		m_InFlightFences.resize(GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT);
		m_ImagesInFlight.resize(m_SwapChainImages.size(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(m_Device.getDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(m_Device.getDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(m_Device.getDevice(), &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS) {

				throw std::runtime_error("VULKAN ERROR: Failed to create synchronization objects for a frame!");
			}
		}
	}

	void Renderer::createCommandBuffers() {
		m_CommandBuffers.resize(GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_Device.getCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

		if (vkAllocateCommandBuffers(m_Device.getDevice(), &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("VULKAN ERROR: Failed to allocate command buffers!");
		}
	}

	VkResult Renderer::submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex) {
		if (m_ImagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences(m_Device.getDevice(), 1, &m_ImagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
		}
		m_ImagesInFlight[*imageIndex] = m_InFlightFences[m_CurrentFrame];

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = buffers;

		VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(m_Device.getDevice(), 1, &m_InFlightFences[m_CurrentFrame]);
		if (vkQueueSubmit(m_Device.getGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) !=
			VK_SUCCESS) {
			throw std::runtime_error("VULKAN ERROR: Failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { m_SwapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = imageIndex;

		auto result = vkQueuePresentKHR(m_Device.getPresentQueue(), &presentInfo);

		m_CurrentFrame = (m_CurrentFrame + 1) % GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT;

		return result;
	}

	VkSurfaceFormatKHR Renderer::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR Renderer::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				//return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D Renderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		#if defined(PW_WIN32)
			if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)()) {
				return capabilities.currentExtent;
			}

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(m_Window.getWidth()),
				static_cast<uint32_t>(m_Window.getHeight())
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		#elif defined(PW_MACOS)
			if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)()) {
				return capabilities.currentExtent;
			}

			int width = m_Window.getWidth();
			int height = m_Window.getWidth();

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		#endif
	}

}
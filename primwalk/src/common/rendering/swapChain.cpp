#include "swapChain.hpp"

#include "../color.hpp"
#include <array>
#include <stdexcept>

namespace pw {

	SwapChain::SwapChain(GraphicsDevice_Vulkan& device) : m_Device(device) {
		createSwapChain();
		createImages();
		createRenderPass();
		createFramebuffers();
		createSyncObjects();
	}

	SwapChain::~SwapChain() {
		// Swap chain cleanup
		for (size_t i = 0; i < m_Images.size(); i++) {
			m_Images[i]->destroy();
		}
		m_Images.clear();

		if (m_SwapChain != nullptr) {
			vkDestroySwapchainKHR(m_Device.getDevice(), m_SwapChain, nullptr);
			m_SwapChain = nullptr;
		}

		for (auto& framebuffer : m_Framebuffers) {
			framebuffer->destroy();
		}

		// Cleanup synchronization objects
		for (size_t i = 0; i < GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(m_Device.getDevice(), m_RenderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(m_Device.getDevice(), m_ImageAvailableSemaphores[i], nullptr);
			vkDestroyFence(m_Device.getDevice(), m_InFlightFences[i], nullptr);
		}
	}

	VkResult SwapChain::acquireNextImage() {
		vkWaitForFences(m_Device.getDevice(), 1, &m_InFlightFences[m_CurrentFrameIndex], VK_TRUE, UINT64_MAX);

		VkResult result = vkAcquireNextImageKHR(
			m_Device.getDevice(),
			m_SwapChain,
			UINT64_MAX,
			m_ImageAvailableSemaphores[m_CurrentFrameIndex],
			VK_NULL_HANDLE,
			&m_CurrentImageIndex);

		return result;
	}

	void SwapChain::beginRenderPass(VkCommandBuffer commandBuffer) {
		m_RenderPass->begin(*m_Framebuffers[m_CurrentImageIndex], commandBuffer);
	}

	void SwapChain::endRenderPass(VkCommandBuffer commandBuffer) {
		m_RenderPass->end(commandBuffer);
	}

	VkResult SwapChain::swapImage(VkCommandBuffer commandBuffer) {
		if (m_ImagesInFlight[m_CurrentImageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences(m_Device.getDevice(), 1, &m_ImagesInFlight[m_CurrentImageIndex], VK_TRUE, UINT64_MAX);
		}
		m_ImagesInFlight[m_CurrentImageIndex] = m_InFlightFences[m_CurrentFrameIndex];

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkCommandBuffer commandBuffersPtr[] = { commandBuffer };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrameIndex] };
		VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrameIndex] };

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = commandBuffersPtr;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(m_Device.getDevice(), 1, &m_InFlightFences[m_CurrentFrameIndex]);
		if (vkQueueSubmit(m_Device.getGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrameIndex]) !=
			VK_SUCCESS) {
			throw std::runtime_error("VULKAN ERROR: Failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_SwapChain;
		presentInfo.pImageIndices = &m_CurrentImageIndex;

		VkResult result = vkQueuePresentKHR(m_Device.getPresentQueue(), &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			//recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("VULKAN ERROR: Failed to present swap chain image!");
		}

		m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT;

		return result;
	}

	void SwapChain::recreateSwapChain() {
		m_Device.waitForGPU();
		cleanupSwapChain();
		createSwapChain();
		createImages();
		createFramebuffers();
	}

	void SwapChain::createSwapChain() {
		SwapChainSupportDetails swapChainSupport = m_Device.getSwapChainSupport();

		m_BestSurfaceFormat = m_Device.chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = m_Device.chooseSwapPresentMode(swapChainSupport.presentModes);
		m_Extent = m_Device.chooseSwapExtent(swapChainSupport.capabilities);

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

	void SwapChain::createImages() {
		uint32_t imageCount;

		// Retrieve swap chain images
		vkGetSwapchainImagesKHR(m_Device.getDevice(), m_SwapChain, &imageCount, nullptr);
		std::vector<VkImage> vulkanSwapchainImages(imageCount);
		vkGetSwapchainImagesKHR(m_Device.getDevice(), m_SwapChain, &imageCount, vulkanSwapchainImages.data());
		m_Images.resize(imageCount);

		for (size_t i = 0; i < imageCount; i++) {
			SwapchainImageInfo swapchainInfo{};
			swapchainInfo.image = vulkanSwapchainImages[i];
			swapchainInfo.format = m_BestSurfaceFormat.format;
			swapchainInfo.width = m_Extent.width;
			swapchainInfo.height = m_Extent.height;
			swapchainInfo.depth = 1;

			m_Images[i] = Image::create(swapchainInfo);
		}
	}

	void SwapChain::createRenderPass() {
		RenderPassAttachment attachment = {
			m_Images[0],
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
		m_RenderPass->setClearColor(0, { 29, 29, 29 });
	}

	void SwapChain::createFramebuffers() {
		m_Framebuffers.resize(m_Images.size());

		// Create framebuffers for image views
		for (size_t i = 0; i < m_Images.size(); i++) {
			FramebufferInfo framebufferInfo = {
				m_Extent.width,
				m_Extent.height,
				m_RenderPass->getVulkanRenderPass(),
				{ { m_Images[i] } }
			};

			m_Framebuffers[i] = std::make_unique<Framebuffer>(framebufferInfo);
		}
	}

	void SwapChain::createSyncObjects() {
		m_ImageAvailableSemaphores.resize(GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT);
		m_RenderFinishedSemaphores.resize(GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT);
		m_InFlightFences.resize(GraphicsDevice_Vulkan::MAX_FRAMES_IN_FLIGHT);
		m_ImagesInFlight.resize(m_Images.size(), VK_NULL_HANDLE);

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

	void SwapChain::cleanupSwapChain() {
		for (auto& framebuffer : m_Framebuffers) {
			framebuffer->destroy();
		}

		for (size_t i = 0; i < m_Images.size(); i++) {
			m_Images[i]->destroy();
		}

		vkDestroySwapchainKHR(m_Device.getDevice(), m_SwapChain, nullptr);
	}

}
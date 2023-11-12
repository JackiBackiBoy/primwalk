#pragma once

#include "graphicsDevice_Vulkan.hpp"
#include "image.hpp"
#include "framebuffer.hpp"
#include "renderpass.hpp"

// std
#include <cstdint>
#include <memory>
#include <vector>

namespace pw {
	class SwapChain {
	public:
		SwapChain(GraphicsDevice_Vulkan& device);
		~SwapChain();

		VkResult acquireNextImage();
		void beginRenderPass(VkCommandBuffer commandBuffer);
		void endRenderPass(VkCommandBuffer commandBuffer);
		VkResult swapImage(VkCommandBuffer commandBuffer);
		void recreateSwapChain();

		inline uint32_t getWidth() const { return m_Extent.width; }
		inline uint32_t getHeight() const { return m_Extent.height; }
		inline size_t getCurrentFrameIndex() const { return m_CurrentFrameIndex; }
		inline RenderPass& getRenderPass() { return *m_RenderPass; }

	private:
		void createSwapChain();
		void createImages();
		void createRenderPass();
		void createFramebuffers();
		void createSyncObjects();
		void cleanupSwapChain();

		GraphicsDevice_Vulkan& m_Device;

		VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
		VkExtent2D m_Extent;
		VkSurfaceFormatKHR m_BestSurfaceFormat;

		std::unique_ptr<RenderPass> m_RenderPass;
		std::vector<std::unique_ptr<Image>> m_Images;
		std::vector<std::unique_ptr<Framebuffer>> m_Framebuffers;
		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkFence> m_InFlightFences;
		std::vector<VkFence> m_ImagesInFlight;

		uint32_t m_CurrentImageIndex = 0;
		size_t m_CurrentFrameIndex = 0;
	};
}
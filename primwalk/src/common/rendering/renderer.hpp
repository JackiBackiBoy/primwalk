#pragma once

// primwalk
#include "../../core.hpp"
#include "../../window.hpp"
#include "../color.hpp"
#include "framebuffer.hpp"
#include "graphicsDevice_Vulkan.hpp"
#include "image.hpp"
#include "renderpass.hpp"

// std
#include <atomic>
#include <memory>
#include <vector>

namespace pw {
	class PW_API Renderer {
	public:
		Renderer(GraphicsDevice_Vulkan& device, Window& window);
		~Renderer();

		VkCommandBuffer beginFrame();
		bool endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void resizeSwapChain(uint32_t width, uint32_t height);

		// Getters
		inline int getFrameIndex() const { return m_CurrentFrameIndex; }
		inline size_t getCurrentFrame() const { return m_CurrentFrame; }
		inline VkRenderPass getSwapChainRenderPass() const { return m_RenderPass->getVulkanRenderPass(); }
		inline uint32_t getSwapChainWidth() const { return m_Extent.width; }
		inline uint32_t getSwapChainHeight() const { return m_Extent.height; }
		static VkSampler m_TextureSampler;

		// Setters
		inline void setClearColor(Color color) { m_ClearColor = color; }

	private:
		void recreateSwapChain();
		void cleanupSwapChain();
		void createSwapChain();
		void createImages();
		void createTextureSampler();
		void createRenderPass();
		void createFramebuffers();
		void createSyncObjects();
		void createCommandBuffers();

		VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		GraphicsDevice_Vulkan& m_Device;
		Window& m_Window;

		VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
		VkExtent2D m_Extent;
		VkSurfaceFormatKHR m_BestSurfaceFormat;
		std::unique_ptr<RenderPass> m_RenderPass;

		std::vector<std::unique_ptr<Image>> m_SwapChainImages;
		std::vector<std::unique_ptr<Framebuffer>> m_SwapChainFramebuffers;
		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkFence> m_InFlightFences;
		std::vector<VkFence> m_ImagesInFlight;
		std::vector<VkCommandBuffer> m_CommandBuffers;
		Color m_ClearColor = { 0, 0, 0 };
		uint32_t m_CurrentImageIndex = 0;
		size_t m_CurrentFrame = 0;
		int m_CurrentFrameIndex{};
	};
}


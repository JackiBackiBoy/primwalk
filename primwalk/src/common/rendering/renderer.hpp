#pragma once

// primwalk
#include "../../core.hpp"
#include "../../window.hpp"
#include "../color.hpp"
#include "framebuffer.hpp"
#include "graphicsDevice_Vulkan.hpp"
#include "swapChain.hpp"

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
		void beginRenderPass(VkCommandBuffer commandBuffer);
		void endRenderPass(VkCommandBuffer commandBuffer);
		void endFrame();
		void resizeSwapChain(uint32_t width, uint32_t height);

		// Getters
		inline size_t getFrameIndex() const { return m_SwapChain->getCurrentFrameIndex(); }
		inline VkRenderPass getVkRenderPass() const { return m_SwapChain->getRenderPass().getVulkanRenderPass(); }
		inline uint32_t getSwapChainWidth() const { return m_SwapChain->getWidth(); }
		inline uint32_t getSwapChainHeight() const { return m_SwapChain->getHeight(); }

	private:
		void createCommandBuffers();

		GraphicsDevice_Vulkan& m_Device;
		Window& m_Window;

		std::unique_ptr<SwapChain> m_SwapChain;
		std::vector<VkCommandBuffer> m_CommandBuffers;
	};
}


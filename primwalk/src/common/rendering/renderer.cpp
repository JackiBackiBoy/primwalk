#include "renderer.hpp"

// std
#include <algorithm>
#include <array>
#include <iostream>

namespace pw {

	Renderer::Renderer(GraphicsDevice_Vulkan& device, Window& window) :
		m_Device(device), m_Window(window) {

		m_SwapChain = std::make_unique<SwapChain>(m_Device);
		createCommandBuffers();
	}

	Renderer::~Renderer() {


		// Renderer cleanup
		vkFreeCommandBuffers(
			m_Device.getDevice(),
			m_Device.getCommandPool(),
			static_cast<uint32_t>(m_CommandBuffers.size()),
			m_CommandBuffers.data());
		m_CommandBuffers.clear();
	}

	VkCommandBuffer Renderer::beginFrame() {
		VkResult result = m_SwapChain->acquireNextImage();

		// TODO: Move to acquireNextImage function in swapchain
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			return nullptr;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("VULKAN ERROR: Failed to acquire swap chain image!");
		}

		auto commandBuffer = m_CommandBuffers[m_SwapChain->getCurrentFrameIndex()];

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("VULKAN ERROR: Failed to begin recording command buffer!");
		}

		return commandBuffer;
	}

	void Renderer::beginRenderPass(VkCommandBuffer commandBuffer) {
		m_SwapChain->beginRenderPass(commandBuffer);
	}

	void Renderer::endRenderPass(VkCommandBuffer commandBuffer) {
		m_SwapChain->endRenderPass(commandBuffer);
	}

	void Renderer::endFrame() {
		auto commandBuffer = m_CommandBuffers[m_SwapChain->getCurrentFrameIndex()];
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("VULKAN ERROR: Failed to record command buffer!");
		}

		// Submit command buffers
		VkResult result = m_SwapChain->swapImage(commandBuffer);
	}

	void Renderer::resizeSwapChain(uint32_t width, uint32_t height) {
		//vkDeviceWaitIdle(m_Device.getDevice());
		#ifdef PW_WIN32
			int clientWidth = 0, clientHeight = 0;
			RECT rc;

			// TODO: Eliminate nasty loop
			do {
				GetClientRect(((WindowWin32&)m_Window).getHandle(), &rc);

				clientWidth = rc.right - rc.left;
				clientHeight = rc.bottom - rc.top;
			} while (clientWidth == 0 || clientHeight == 0);
		#endif

		m_SwapChain->recreateSwapChain();
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

}
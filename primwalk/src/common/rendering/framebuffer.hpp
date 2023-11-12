#pragma once

// primwalk
#include "../../core.hpp"
#include "image.hpp"

// std
#include <cstdint>
#include <memory>
#include <vector>

// vendor
#include <vulkan/vulkan.h>

namespace pw {
	struct PW_API FramebufferInfo {
		uint32_t width = 0;
		uint32_t height = 0;
		VkRenderPass renderPass = VK_NULL_HANDLE;
		const std::vector<std::reference_wrapper<std::unique_ptr<Image>>>& attachments;
	};

	class PW_API Framebuffer {
	public:
		Framebuffer(const FramebufferInfo& createInfo);
		~Framebuffer() = default;

		/* Getters */
		[[nodiscard]] inline uint32_t getWidth() const { return m_Width; }
		[[nodiscard]] inline uint32_t getHeight() const { return m_Height; }
		[[nodiscard]] inline VkFramebuffer getVkFramebuffer() const { return m_FrameBuffer; }

		void destroy();
	
	private:
		VkFramebuffer m_FrameBuffer = VK_NULL_HANDLE;
		uint32_t m_Width, m_Height;
	};
}


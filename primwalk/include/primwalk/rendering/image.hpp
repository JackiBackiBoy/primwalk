#pragma once

// primwalk
#include "primwalk/core.hpp"

// std
#include <memory>

// vendor
#include <vulkan/vulkan.h>

namespace pw {
	struct PW_API ImageInfo {
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t depth = 1;
		VkImageType type = VK_IMAGE_TYPE_2D;
		VkFormat format = VK_FORMAT_UNDEFINED;
		VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
		VkSampleCountFlagBits sampling = VK_SAMPLE_COUNT_1_BIT;
		VkImageUsageFlags usage = 0;
		VkImageCreateFlags createFlags = 0;

		uint32_t layerCount = 1;
		bool generateMipMaps = false;
	};

	struct PW_API SwapchainImageInfo
	{
		VkImage image = VK_NULL_HANDLE;
		VkFormat format = VK_FORMAT_UNDEFINED;
		VkSampleCountFlagBits sampling = VK_SAMPLE_COUNT_1_BIT;
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t depth = 1;
	};

	class PW_API Image {
	public:
		Image(const ImageInfo& createInfo);
		static std::unique_ptr<Image> create(const ImageInfo& createInfo);

		Image(const SwapchainImageInfo& createInfo);
		static std::unique_ptr<Image> create(const SwapchainImageInfo& createInfo);

		~Image();
		void destroy();

		/* Getters */
		[[nodiscard]] inline uint32_t getWidth() const { return m_Width; }
		[[nodiscard]] inline uint32_t getHeight() const { return m_Height; }
		[[nodiscard]] inline uint32_t getDepth() const { return m_Depth; }
		[[nodiscard]] inline uint32_t getLayerCount() const { return m_LayerCount; }
		[[nodiscard]] inline VkFormat getFormat() const { return m_Format; }
		[[nodiscard]] inline VkSampleCountFlagBits getSampling() const { return m_Sampling; }
		[[nodiscard]] inline VkImageUsageFlags getUsage() const { return m_Usage; }
		[[nodiscard]] inline VkImageCreateFlags getCreateFlags() const { return m_Flags; }
		[[nodiscard]] inline VkImage getVulkanImage() const { return m_Image; }
		[[nodiscard]] inline VkImageView getVulkanImageView() const { return m_ImageView; }

		/* Setters */
		void transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout);

	private:
		void createImageView();

		uint32_t m_Width;
		uint32_t m_Height;
		uint32_t m_Depth;
		uint32_t m_MipLevels = 1;
		uint32_t m_LayerCount = 1;
		bool m_SwapchainImage = false;

		VkImageType m_Type;
		VkFormat m_Format;
		VkSampleCountFlagBits m_Sampling;
		VkImageUsageFlags m_Usage;
		VkImageCreateFlags m_Flags = 0;
		VkImageLayout m_Layout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImage m_Image = VK_NULL_HANDLE;
		VkImageView m_ImageView = VK_NULL_HANDLE;
		VkDeviceMemory m_ImageMemory = VK_NULL_HANDLE;
	};
}


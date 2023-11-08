#include "buffer.hpp"

namespace pw {

	Buffer::Buffer(
		GraphicsDevice_Vulkan& device,
		VkDeviceSize instanceSize,
		uint32_t instanceCount,
		VkBufferUsageFlags usageFlags,
		VkMemoryPropertyFlags memoryPropertyFlags,
		VkDeviceSize minOffsetAlignment) :
	m_Device(device), m_InstanceSize(instanceSize), m_InstanceCount(instanceCount),
	m_UsageFlags(usageFlags), m_MemoryPropertyFlags(memoryPropertyFlags) {
		m_AlignmentSize = getAlignment(instanceSize, minOffsetAlignment);
		m_BufferSize = m_AlignmentSize * instanceCount;

		m_Device.createBuffer(m_BufferSize, m_UsageFlags, m_MemoryPropertyFlags, m_Buffer, m_Memory);
	}

	Buffer::~Buffer() {
		unmap();
		vkDestroyBuffer(m_Device.getDevice(), m_Buffer, nullptr);
		vkFreeMemory(m_Device.getDevice(), m_Memory, nullptr);
	}

	void Buffer::map(VkDeviceSize size /*= VK_WHOLE_SIZE*/, VkDeviceSize offset /*= 0*/) {
		vkMapMemory(m_Device.getDevice(), m_Memory, offset, size, 0, &m_Mapped);
	}

	void Buffer::unmap() {
		if (m_Mapped) {
			vkUnmapMemory(m_Device.getDevice(), m_Memory);
			m_Mapped = nullptr;
		}
	}

	void Buffer::writeToBuffer(void* data, VkDeviceSize size /*= VK_WHOLE_SIZE*/, VkDeviceSize offset /*= 0*/) {
		if (size == VK_WHOLE_SIZE) {
			memcpy(m_Mapped, data, m_BufferSize);
			return;
		}

		char* memOffset = (char*)m_Mapped;
		memOffset += offset;
		memcpy(memOffset, data, size);
	}

	void Buffer::copyToImage(VkCommandBuffer commandBuffer, Image& image) {
		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = image.getLayerCount();
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { image.getWidth(), image.getHeight(), 1 };

		vkCmdCopyBufferToImage(
			commandBuffer,
			m_Buffer,
			image.getVulkanImage(),
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region);
	}

	VkDescriptorBufferInfo Buffer::getDescriptorInfo(VkDeviceSize size /*= VK_WHOLE_SIZE*/, VkDeviceSize offset /*= 0*/) {
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_Buffer;
		bufferInfo.offset = offset;
		bufferInfo.range = size;

		return bufferInfo;
	}

	VkDeviceSize Buffer::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) {
		if (minOffsetAlignment > 0) {
			return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
		}

		return instanceSize;
	}

}
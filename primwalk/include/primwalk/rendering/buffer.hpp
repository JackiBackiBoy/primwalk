#pragma once

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/rendering/graphicsDevice.hpp"
#include "primwalk/rendering/image.hpp"

// vendor
#include <vulkan/vulkan.h>

namespace pw {
  class PW_API Buffer {
    public:
      Buffer(
        GraphicsDevice_Vulkan& device,
        VkDeviceSize instanceSize,
        uint32_t instanceCount,
        VkBufferUsageFlags usageFlags,
        VkMemoryPropertyFlags memoryPropertyFlags,
        VkDeviceSize minOffsetAlignment = 1);
      ~Buffer();

      void map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
      void unmap();
      void writeToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
      void copyToImage(VkCommandBuffer commandBuffer, Image& image);

      // Getters
      VkBuffer getBuffer();
      VkDeviceSize getBufferSize() const;
      VkBufferUsageFlags getUsageFlags() const;
      VkMemoryPropertyFlags getMemoryPropertyFlags() const;
      VkDescriptorBufferInfo getDescriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    private:
      static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

      GraphicsDevice_Vulkan& m_Device;
      void* m_Mapped = nullptr;
      VkBuffer m_Buffer = VK_NULL_HANDLE;
      VkDeviceMemory m_Memory = VK_NULL_HANDLE;

      VkDeviceSize m_BufferSize;
      VkDeviceSize m_InstanceSize;
      uint32_t m_InstanceCount;
      VkDeviceSize m_AlignmentSize;
      VkBufferUsageFlags m_UsageFlags;
      VkMemoryPropertyFlags m_MemoryPropertyFlags;
  };
}


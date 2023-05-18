#ifndef FZ_DESCRIPTOR_POOL_HEADER
#define FZ_DESCRITPOR_POOL_HEADER

// FZUI
#include "fzui/core.hpp"
#include "fzui/rendering/graphicsDevice.hpp"

// std
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

namespace fz {
  // ******** Descriptor Set Layout ********
  class FZ_API DescriptorSetLayout {
    public:
      // Nested builder class
      class Builder {
        public:
          Builder(GraphicsDevice_Vulkan& device);
          ~Builder() {};

          Builder& addBinding(
            uint32_t binding,
            VkDescriptorType descriptorType,
            VkShaderStageFlags stageFlags,
            uint32_t count = 1);
          std::unique_ptr<DescriptorSetLayout> build() const;

        private:
          GraphicsDevice_Vulkan& m_Device;
          std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_Bindings;
      };

      DescriptorSetLayout(
        GraphicsDevice_Vulkan& device,
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);

      ~DescriptorSetLayout();

      // Getters
      VkDescriptorSetLayout getDescriptorSetLayout() const;

    private:
      GraphicsDevice_Vulkan& m_Device;
      VkDescriptorSetLayout m_DescriptorSetLayout;
      std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_Bindings;

      friend class DescriptorWriter;
  };

  // ******** Descriptor Pool ********
  class FZ_API DescriptorPool {
    public:

      // Nested builder class
      class Builder {
        public:
          Builder(GraphicsDevice_Vulkan& device);
          ~Builder() {};

          Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
          Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
          Builder& setMaxSets(uint32_t count);
          std::unique_ptr<DescriptorPool> build() const;

        private:
          GraphicsDevice_Vulkan& m_Device;
          uint32_t m_MaxSets = 100;
          VkDescriptorPoolCreateFlags m_PoolFlags = 0;
          std::vector<VkDescriptorPoolSize> m_PoolSizes;
      };

      DescriptorPool(
        GraphicsDevice_Vulkan& device,
        uint32_t maxSets,
        VkDescriptorPoolCreateFlags poolFlags,
        const std::vector<VkDescriptorPoolSize>& poolSizes);
      ~DescriptorPool();

      bool allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

    private:
      GraphicsDevice_Vulkan& m_Device;
      VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;

      friend class DescriptorWriter;
  };

  // ******** Descriptor Writer ********
  class FZ_API DescriptorWriter {
    public:
      DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool);

      DescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);

      bool build(VkDescriptorSet& set);
      void overwrite(VkDescriptorSet& set);

    private:
      std::vector<VkWriteDescriptorSet> m_Writes;
      DescriptorSetLayout& m_SetLayout;
      DescriptorPool& m_Pool;
  };
}
#endif
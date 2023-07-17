// primwalk
#include "primwalk/rendering/descriptors.hpp"

// std
#include <stdexcept>
#include <iostream>

namespace pw {
  // ******** Descriptor Set Layout Builder ********

  DescriptorSetLayout::Builder::Builder(GraphicsDevice_Vulkan& device) :
    m_Device{device}
  {

  }

  DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::addBinding(
    uint32_t binding, VkDescriptorType descriptorType,
    VkShaderStageFlags stageFlags, uint32_t count /*= 1*/,
    VkDescriptorBindingFlags bindingFlags /*= 0*/)
  {
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = descriptorType;
    layoutBinding.descriptorCount = count;
    layoutBinding.stageFlags = stageFlags;

    m_Bindings[binding] = layoutBinding;
    m_BindingFlags[binding] = bindingFlags;
    return *this;
  }

  DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::setLayoutFlags(VkDescriptorSetLayoutCreateFlags layoutFlags) {
    m_LayoutFlags = layoutFlags;
    return *this;
  }

  std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::build() const
  {
    return std::make_unique<DescriptorSetLayout>(m_Device, m_Bindings, m_BindingFlags, m_LayoutFlags);
  }

  // ******** Descriptor Set Layout ********

  DescriptorSetLayout::DescriptorSetLayout(
    GraphicsDevice_Vulkan& device,
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings,
    std::unordered_map<uint32_t, VkDescriptorBindingFlags> bindingFlags,
    VkDescriptorSetLayoutCreateFlags layoutFlags) :
    m_Device{device}, m_Bindings{bindings}, m_BindingFlags{bindingFlags}, m_LayoutFlags{layoutFlags}
  {

    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
    std::vector<VkDescriptorBindingFlags> setLayoutBindingFlags{};

    for (auto& kv : bindings) {
      setLayoutBindings.push_back(kv.second);
    }

    for (auto& kv: bindingFlags) {
      setLayoutBindingFlags.push_back(kv.second);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
    layoutInfo.pBindings = setLayoutBindings.data();
    layoutInfo.flags = m_LayoutFlags;

    VkDescriptorSetLayoutBindingFlagsCreateInfo flagInfo{};
    flagInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    flagInfo.bindingCount = static_cast<uint32_t>(setLayoutBindingFlags.size());
    flagInfo.pBindingFlags = setLayoutBindingFlags.data();

    if (setLayoutBindingFlags.size() > 0) {
      layoutInfo.pNext = &flagInfo;
    }

    if (vkCreateDescriptorSetLayout(m_Device.getDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS) {
      throw std::runtime_error("VULKAN ERROR: Failed to create descriptor set layout!");
    }
  }

  DescriptorSetLayout::~DescriptorSetLayout()
  {
    vkDestroyDescriptorSetLayout(m_Device.getDevice(), m_DescriptorSetLayout, nullptr);
  }


  VkDescriptorSetLayout DescriptorSetLayout::getDescriptorSetLayout() const
  {
    return m_DescriptorSetLayout;
  }

  // ******** Descriptor Pool Builder ********

  DescriptorPool::Builder::Builder(GraphicsDevice_Vulkan& device) :
    m_Device{device}
  {

  }

  DescriptorPool::Builder& DescriptorPool::Builder::addPoolSize(VkDescriptorType descriptorType, uint32_t count)
  {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = descriptorType;
    poolSize.descriptorCount = count;

    m_PoolSizes.push_back(poolSize);
    return *this;
  }

  DescriptorPool::Builder& DescriptorPool::Builder::setPoolFlags(VkDescriptorPoolCreateFlags flags)
  {
    m_PoolFlags = flags;
    return *this;
  }

  DescriptorPool::Builder& DescriptorPool::Builder::setMaxSets(uint32_t count)
  {
    m_MaxSets = count;
    return *this;
  }

  std::unique_ptr<DescriptorPool> DescriptorPool::Builder::build() const
  {
    return std::make_unique<DescriptorPool>(m_Device, m_MaxSets, m_PoolFlags, m_PoolSizes);
  }

  // ******** Descriptor Pool ********

  DescriptorPool::DescriptorPool(
    GraphicsDevice_Vulkan& device,
    uint32_t maxSets,
    VkDescriptorPoolCreateFlags poolFlags,
    const std::vector<VkDescriptorPoolSize>& poolSizes) :
    m_Device{device}
  {
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = maxSets;
    poolInfo.flags = poolFlags;

    if (vkCreateDescriptorPool(m_Device.getDevice(), &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS) {
      throw std::runtime_error("VULKAN ERROR: Failed to create descriptor pool!");
    }
  }

  DescriptorPool::~DescriptorPool()
  {
    vkDestroyDescriptorPool(m_Device.getDevice(), m_DescriptorPool, nullptr);
  }

  bool DescriptorPool::allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const
  {
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_DescriptorPool;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    allocInfo.descriptorSetCount = 1;

    if (vkAllocateDescriptorSets(m_Device.getDevice(), &allocInfo, &descriptor) != VK_SUCCESS) {
      return false;
    }

    return true;
  }

  // ******** Descriptor Writer ********

  DescriptorWriter::DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool) :
    m_SetLayout{setLayout}, m_Pool{pool}
  {

  }

  DescriptorWriter& DescriptorWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo)
  {
    auto& bindingDescription = m_SetLayout.m_Bindings[binding];

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.descriptorCount = 1;
    write.dstBinding = binding;
    write.pBufferInfo = bufferInfo;

    m_Writes.push_back(write);
    return *this;
  }

  DescriptorWriter& DescriptorWriter::writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo, uint32_t index)
  {
    auto& bindingDescription = m_SetLayout.m_Bindings[binding];

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.descriptorCount = 1;
    write.dstBinding = binding;
    write.dstArrayElement = index;
    write.pImageInfo = imageInfo;

    m_Writes.push_back(write);
    return *this;
  }

  bool DescriptorWriter::build(VkDescriptorSet& set)
  {
    bool success = m_Pool.allocateDescriptor(m_SetLayout.getDescriptorSetLayout(), set);
    if (!success) {
      std::cout << "Failed descriptor set allocation!\n";
      return false;
    }
    overwrite(set);
    return true;
  }

  void DescriptorWriter::overwrite(VkDescriptorSet& set)
  {
    for (auto& write : m_Writes) {
      write.dstSet = set;
    }

    vkUpdateDescriptorSets(m_Pool.m_Device.getDevice(), m_Writes.size(), m_Writes.data(), 0, nullptr);
  }

}
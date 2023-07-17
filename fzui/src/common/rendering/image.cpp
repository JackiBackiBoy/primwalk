#include "fzui/rendering/image.hpp"
#include "fzui/rendering/graphicsDevice.hpp"

// std
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace fz {

  Image::Image(const ImageInfo& createInfo) :
    m_Width(createInfo.width), m_Height(createInfo.height), m_Depth(createInfo.depth),
    m_LayerCount(createInfo.layerCount), m_Usage(createInfo.usage),
    m_Sampling(createInfo.sampling), m_Format(createInfo.format),
    m_Type(createInfo.type), m_Flags(createInfo.createFlags)
  {
    GraphicsDevice_Vulkan* device = fz::GetDevice();

    // Create info
    VkImageCreateInfo vkImageCreateInfo{};
    vkImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    vkImageCreateInfo.imageType = createInfo.type;

    vkImageCreateInfo.extent.width = m_Width;
    vkImageCreateInfo.extent.height = m_Height;
    vkImageCreateInfo.extent.depth = m_Depth;

    if (createInfo.generateMipMaps) {
      m_MipLevels = static_cast<uint32_t>(
        std::floor(std::log2(std::max(m_Width, m_Height))) + 1
      );
    }

    vkImageCreateInfo.mipLevels = m_MipLevels;
    vkImageCreateInfo.arrayLayers = createInfo.layerCount;
    vkImageCreateInfo.format = m_Format;
    vkImageCreateInfo.tiling = createInfo.tiling;
    vkImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkImageCreateInfo.usage = createInfo.usage;
    vkImageCreateInfo.samples = m_Sampling;
    vkImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkImageCreateInfo.flags = createInfo.createFlags;

    // Set up image allocation info
    if (vkCreateImage(device->getDevice(), &vkImageCreateInfo, nullptr, &m_Image) != VK_SUCCESS) {
      throw std::runtime_error("VULKAN ERROR: Failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device->getDevice(), m_Image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = device->findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(device->getDevice(), &allocInfo, nullptr, &m_ImageMemory) != VK_SUCCESS) {
      throw std::runtime_error("VULKAN ERROR: Failed to allocate image memory!");
    }

    if (vkBindImageMemory(device->getDevice(), m_Image, m_ImageMemory, 0) != VK_SUCCESS) {
      throw std::runtime_error("VULKAN ERROR: Failed to bind image memory!");
    }

    createImageView();
  }

  Image::Image(const SwapchainImageInfo& createInfo) :
    m_Image(createInfo.image), m_Format(createInfo.format), m_Usage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT),
    m_Sampling(createInfo.sampling), m_Width(createInfo.width), m_Height(createInfo.height),
    m_Depth(createInfo.depth), m_SwapchainImage(true)
  {
    createImageView();
  }

  std::unique_ptr<Image> Image::create(const ImageInfo& createInfo)
  {
    return std::make_unique<Image>(createInfo);
  }

  std::unique_ptr<Image> Image::create(const SwapchainImageInfo& createInfo)
  {
    return std::make_unique<Image>(createInfo);
  }

  Image::~Image()
  {
    //GraphicsDevice_Vulkan* device = fz::GetDevice();

    //vkDestroyImageView(device->getDevice(), m_ImageView, nullptr);
    //vkDestroyImage(device->getDevice(), m_Image, nullptr);
    //vkFreeMemory(device->getDevice(), m_ImageMemory, nullptr);
  }

  void Image::destroy()
  {
    GraphicsDevice_Vulkan* device = fz::GetDevice();

    vkDestroyImageView(device->getDevice(), m_ImageView, nullptr);

    if (!m_SwapchainImage) {
      vkDestroyImage(device->getDevice(), m_Image, nullptr);
      vkFreeMemory(device->getDevice(), m_ImageMemory, nullptr);
    }
  }

  void Image::transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout)
  {
    if (m_Layout == newLayout) { return; }

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = m_Layout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_Image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = m_MipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = m_LayerCount;
    barrier.srcAccessMask = 0; // TODO
    barrier.dstAccessMask = 0; // TODO

    // Transition barrier masks
    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (m_Layout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
      barrier.srcAccessMask = 0;
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

      sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (m_Layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

      sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (m_Layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
      barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

      sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
      destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else {
      throw std::invalid_argument("VULKAN ERROR: Unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
      commandBuffer,
      sourceStage, destinationStage,
      0,
      0, nullptr,
      0, nullptr,
      1, &barrier
    );

    m_Layout = newLayout;
  }

  void Image::createImageView()
  {
    GraphicsDevice_Vulkan* device = fz::GetDevice();

    VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
    // TODO: Check for other applicable view types

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_Image;
    viewInfo.viewType = viewType;
    viewInfo.format = m_Format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // TODO: make dynamic
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = m_MipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = m_LayerCount;

    if (vkCreateImageView(device->getDevice(), &viewInfo, nullptr, &m_ImageView) != VK_SUCCESS) {
      throw std::runtime_error("VULKAN ERROR: Failed to create image view!");
    }
  }

}
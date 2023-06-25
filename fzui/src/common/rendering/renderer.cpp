#include "fzui/rendering/renderer.hpp"
#include "fzui/rendering/graphicsDevice.hpp"

// std
#include <algorithm>
#include <iostream>

namespace fz {

  Renderer::Renderer(Window& window) :
    m_Window(window)
  {
    // Swap chain
    createSwapChain();
    createImageViews();
    createRenderPass();
    createFramebuffers();
    createTextureSampler();
    createSyncObjects();

    // Command buffers
    createCommandBuffers();
  }

  Renderer::~Renderer()
  {
    GraphicsDevice_Vulkan* device = fz::GetDevice();

    // TODO: Check order of deletion of sampler
    vkDestroySampler(device->getDevice(), m_TextureSampler, nullptr);

    // Swap chain cleanup
    for (auto imageView : m_SwapChainImageViews) {
      vkDestroyImageView(device->getDevice(), imageView, nullptr);
    }
    m_SwapChainImageViews.clear();

    if (m_SwapChain != nullptr) {
      vkDestroySwapchainKHR(device->getDevice(), m_SwapChain, nullptr);
      m_SwapChain = nullptr;
    }

    for (auto& framebuffer : m_SwapChainFramebuffers) {
      framebuffer->destroy();
    }

    vkDestroyRenderPass(device->getDevice(), m_RenderPass, nullptr);

    // Cleanup synchronization objects
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      vkDestroySemaphore(device->getDevice(), m_RenderFinishedSemaphores[i], nullptr);
      vkDestroySemaphore(device->getDevice(), m_ImageAvailableSemaphores[i], nullptr);
      vkDestroyFence(device->getDevice(), m_InFlightFences[i], nullptr);
    }

    // Renderer cleanup
    vkFreeCommandBuffers(
      device->getDevice(),
      device->getCommandPool(),
      static_cast<uint32_t>(m_CommandBuffers.size()),
      m_CommandBuffers.data());
    m_CommandBuffers.clear();
  }

  VkCommandBuffer Renderer::beginFrame()
  {
    GraphicsDevice_Vulkan* device = fz::GetDevice();
    vkWaitForFences(device->getDevice(), 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

    // Acquire image from the swap chain
    VkResult result = vkAcquireNextImageKHR(device->getDevice(), m_SwapChain, UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &m_CurrentImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
      recreateSwapChain();
      return nullptr;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
      throw std::runtime_error("VULKAN ERROR: Failed to acquire swap chain image!");
    }

    auto commandBuffer = m_CommandBuffers[m_CurrentFrameIndex];

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
      throw std::runtime_error("VULKAN ERROR: Failed to begin recording command buffer!");
    }

    return commandBuffer;
  }

  bool Renderer::endFrame()
  {
    bool ret = false;

    auto commandBuffer = m_CommandBuffers[m_CurrentFrameIndex];
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
      throw std::runtime_error("VULKAN ERROR: Failed to record command buffer!");
    }

    auto result = submitCommandBuffers(&commandBuffer, &m_CurrentImageIndex);


    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      m_FramebufferResized.load(std::memory_order_relaxed)) {
      m_FramebufferResized.store(false, std::memory_order_relaxed);
      recreateSwapChain();
      ret = true;
    }
    else if (result != VK_SUCCESS) {
      throw std::runtime_error("VULKAN ERROR: Failed to present swap chain image!");
    }

    //m_IsFrameStarted = false;
    m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
    return ret;
  }

  void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
  {
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_RenderPass;
    renderPassInfo.framebuffer = m_SwapChainFramebuffers[m_CurrentImageIndex]->getRawFramebuffer();
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = m_SwapChainExtent;

    std::array<VkClearValue, 2> clearValues{};
    glm::vec4 normColor = Color::normalize(m_ClearColor);
    clearValues[0].color = { normColor.r, normColor.g, normColor.b, 1.0f };
    clearValues[1].depthStencil = { 1.0f, 0 };
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_SwapChainExtent.width);
    viewport.height = static_cast<float>(m_SwapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{ {0, 0}, m_SwapChainExtent };
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
  }

  void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
  {
    vkCmdEndRenderPass(commandBuffer);
  }

  int Renderer::getFrameIndex() const
  {
    return m_CurrentFrameIndex;
  }

  size_t Renderer::getCurrentFrame() const
  {
    return m_CurrentFrame;
  }

  VkRenderPass Renderer::getSwapChainRenderPass() const
  {
    return m_RenderPass;
  }

  void Renderer::recreateSwapChain()
  {
    GraphicsDevice_Vulkan* device = fz::GetDevice();

    #ifdef FZ_WIN32
    int width = 0, height = 0;
    RECT rc;

    do {
      GetClientRect(((WindowWin32&)m_Window).getHandle(), &rc);
      width = rc.right - rc.left;
      height = rc.bottom - rc.top;
    } while (width == 0 || height == 0);
    #endif

    vkDeviceWaitIdle(device->getDevice());

    cleanupSwapChain();
    createSwapChain();
    createImageViews();
    createFramebuffers();
  }

  uint32_t Renderer::getSwapChainWidth() const {
    return m_SwapChainExtent.width;
  }

  uint32_t Renderer::getSwapChainHeight() const {
    return m_SwapChainExtent.height;
  }

  VkSampler Renderer::m_TextureSampler = VK_NULL_HANDLE;

  void Renderer::cleanupSwapChain() {
    GraphicsDevice_Vulkan* device = fz::GetDevice();

    // Destroy framebuffers
    for (auto& framebuffer : m_SwapChainFramebuffers) {
      framebuffer->destroy();
    }

    // Destroy image views
    for (auto imageView : m_SwapChainImageViews) {
      vkDestroyImageView(device->getDevice(), imageView, nullptr);
    }

    vkDestroySwapchainKHR(device->getDevice(), m_SwapChain, nullptr);
  }

  void Renderer::createSwapChain()
  {
    GraphicsDevice_Vulkan* device = fz::GetDevice();

    //SwapChainSupportDetails swapChainSupport = querySwapChainSupport(m_PhysicalDevice);
    SwapChainSupportDetails swapChainSupport = device->getSwapChainSupport();

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    // Decide swap chain image count
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
      imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    // Creation info
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = device->getSurface();
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Swap chain handling specification across queue families
    QueueFamilyIndices indices = device->findPhysicalQueueFamilies();
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily) {
      createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      createInfo.queueFamilyIndexCount = 2;
      createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
      createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      createInfo.queueFamilyIndexCount = 0; // optional
      createInfo.pQueueFamilyIndices = nullptr; // optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device->getDevice(), &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS) {
      throw std::runtime_error("VULKAN ERROR: Failed to create swap chain!");
    }

    // Retrieve swap chain images
    vkGetSwapchainImagesKHR(device->getDevice(), m_SwapChain, &imageCount, nullptr);
    m_SwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device->getDevice(), m_SwapChain, &imageCount, m_SwapChainImages.data());

    m_SwapChainImageFormat = surfaceFormat.format;
    m_SwapChainExtent = extent;
  }

  void Renderer::createImageViews()
  {
    GraphicsDevice_Vulkan* device = fz::GetDevice();

    m_SwapChainImageViews.resize(m_SwapChainImages.size());

    // Create an accompanying image view for each swap chain image
    for (size_t i = 0; i < m_SwapChainImages.size(); i++) {
      VkImageViewCreateInfo createInfo{};
      createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      createInfo.image = m_SwapChainImages[i];
      createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
      createInfo.format = m_SwapChainImageFormat;

      // Color channel mapping
      createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

      // Describe image's purpose
      createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      createInfo.subresourceRange.baseMipLevel = 0;
      createInfo.subresourceRange.levelCount = 1;
      createInfo.subresourceRange.baseArrayLayer = 0;
      createInfo.subresourceRange.layerCount = 1;

      // Create the image view
      if (vkCreateImageView(device->getDevice(), &createInfo, nullptr, &m_SwapChainImageViews[i]) != VK_SUCCESS) {
        throw std::runtime_error("VULKAN ERROR: Failed to create image view!");
      }
    }
  }

  void Renderer::createTextureSampler()
  {
    GraphicsDevice_Vulkan* device = fz::GetDevice();

    // Device properties
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(device->getPhysicalDevice(), &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(device->getDevice(), &samplerInfo, nullptr, &m_TextureSampler) != VK_SUCCESS) {
      throw std::runtime_error("VULKAN ERROR: Failed to create texture sampler!");
    }
  }

  void Renderer::createRenderPass()
  {
    GraphicsDevice_Vulkan* device = fz::GetDevice();

    // Attachment description
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_SwapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Attachment references
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Subpass
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    // Dependency
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // Render pass
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device->getDevice(), &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS) {
      throw std::runtime_error("VULKAN ERROR: Failed to create render pass!");
    }
  }

  void Renderer::createFramebuffers() {
    GraphicsDevice_Vulkan* device = fz::GetDevice();

    m_SwapChainFramebuffers.resize(m_SwapChainImages.size());

    // Create framebuffers for image views
    for (size_t i = 0; i < m_SwapChainImages.size(); i++) {
      std::vector<VkImageView> attachments = {
        m_SwapChainImageViews[i]
      };

      FramebufferInfo framebufferInfo;
      framebufferInfo.width = m_SwapChainExtent.width;
      framebufferInfo.height = m_SwapChainExtent.height;
      framebufferInfo.renderPass = m_RenderPass;
      framebufferInfo.attachments = attachments;
      m_SwapChainFramebuffers[i] = std::make_unique<Framebuffer>(framebufferInfo);
    }
  }

  void Renderer::createSyncObjects() {
    GraphicsDevice_Vulkan* device = fz::GetDevice();

    m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    m_ImagesInFlight.resize(m_SwapChainImages.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      if (vkCreateSemaphore(device->getDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
        vkCreateSemaphore(device->getDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS ||
        vkCreateFence(device->getDevice(), &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS) {
        throw std::runtime_error("VULKAN ERROR: Failed to create synchronization objects for a frame!");
      }
    }
  }

  void Renderer::createCommandBuffers() {
    GraphicsDevice_Vulkan* device = fz::GetDevice();

    m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = device->getCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

    if (vkAllocateCommandBuffers(device->getDevice(), &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS) {
      throw std::runtime_error("VULKAN ERROR: Failed to allocate command buffers!");
    }
  }

  VkResult Renderer::submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex) {
    GraphicsDevice_Vulkan* device = fz::GetDevice();

    if (m_ImagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
      vkWaitForFences(device->getDevice(), 1, &m_ImagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
    }
    m_ImagesInFlight[*imageIndex] = m_InFlightFences[m_CurrentFrame];

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = buffers;

    VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(device->getDevice(), 1, &m_InFlightFences[m_CurrentFrame]);
    if (vkQueueSubmit(device->getGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) !=
      VK_SUCCESS) {
      throw std::runtime_error("VULKAN ERROR: Failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { m_SwapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = imageIndex;

    auto result = vkQueuePresentKHR(device->getPresentQueue(), &presentInfo);

    m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    return result;
  }

  VkSurfaceFormatKHR Renderer::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
  {
    for (const auto& availableFormat : availableFormats) {
      if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM) {
        return availableFormat;
      }
    }

    return availableFormats[0];
  }

  VkPresentModeKHR Renderer::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
  {
    for (const auto& availablePresentMode : availablePresentModes) {
      if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
        //return availablePresentMode;
      }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
  }

  VkExtent2D Renderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
  {
    if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)()) {
      return capabilities.currentExtent;
    }
    #ifdef FZ_WIN32
    else {
      RECT rc;
      GetClientRect(((WindowWin32&)m_Window).getHandle(), &rc);

      int width = rc.right - rc.left;
      int height = rc.bottom - rc.top;

      VkExtent2D actualExtent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
      };

      actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
      actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

      return actualExtent;
    }
    #endif
  }

}
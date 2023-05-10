// FZUI
#include "fzui/rendering/graphicsDevice.hpp"

// std
#include <algorithm>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>

// vendor
#include <glm/gtc/matrix_transform.hpp>

namespace fz {
  // ------ DirectX 12 ------

  // ------ Vulkan ------
  GraphicsDevice_Vulkan::GraphicsDevice_Vulkan(HWND hWnd) {
    m_Handle = hWnd;

    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createCommandPool();

    // Cue swap chain creation
    //createRenderPass(); // TODO: Put into swap chain class
    //createSyncObjects();

    //createVertexBuffer();
    //createIndexBuffer();
    //createUniformBuffers();
    //createDescriptorPool();
    //createCommandBuffers();
  }

  GraphicsDevice_Vulkan::~GraphicsDevice_Vulkan() {
    vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
    vkDestroyDevice(m_Device, nullptr);

    if (m_EnableValidationLayers) {
      destroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    vkDestroyInstance(m_Instance, nullptr);
  }

  // Getters
  VkDevice GraphicsDevice_Vulkan::getDevice() {
    return m_Device;
  }

  VkCommandPool GraphicsDevice_Vulkan::getCommandPool()
  {
    return m_CommandPool;
  }

  VkSurfaceKHR GraphicsDevice_Vulkan::getSurface()
  {
    return m_Surface;
  }

  VkQueue GraphicsDevice_Vulkan::getGraphicsQueue()
  {
    return m_GraphicsQueue;
  }

  VkQueue GraphicsDevice_Vulkan::getPresentQueue()
  {
    return m_PresentQueue;
  }

  SwapChainSupportDetails GraphicsDevice_Vulkan::getSwapChainSupport()
  {
    return querySwapChainSupport(m_PhysicalDevice);
  }

  QueueFamilyIndices GraphicsDevice_Vulkan::findPhysicalQueueFamilies()
  {
    return findQueueFamilies(m_PhysicalDevice);
  }

  void GraphicsDevice_Vulkan::createInstance() {
    // Check validation layer support
    if (m_EnableValidationLayers && !checkValidationLayerSupport()) {
      throw std::runtime_error("VULKAN ERROR: Validation layers requested, but not available!");
    }

    // Application info (optional)
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Renderer";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    // Instance creation info
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    
    auto requiredExtensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();

    // Debug messenger
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

    // Include validation layer data if requested
    if (m_EnableValidationLayers) {
      createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
      createInfo.ppEnabledLayerNames = m_ValidationLayers.data();

      populateDebugMessengerCreateInfo(debugCreateInfo);
      createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else {
      createInfo.enabledLayerCount = 0;
      createInfo.pNext = nullptr;
    }

    // Create instance
    if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS) {
      throw std::runtime_error("VULKAN ERROR: Failed to create Vulkan instance!");
    }
  }

  void GraphicsDevice_Vulkan::setupDebugMessenger()
  {
    if (!m_EnableValidationLayers) {
      return;
    }

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (createDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS) {
      throw std::runtime_error("VULKAN ERROR: Failed debug messenger setup!");
    }
  }

  void GraphicsDevice_Vulkan::createSurface()
  {
    // TODO: Make cross platform
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = m_Handle;
    createInfo.hinstance = GetModuleHandle(0);

    if (vkCreateWin32SurfaceKHR(m_Instance, &createInfo, nullptr, &m_Surface) != VK_SUCCESS) {
      throw std::runtime_error("VULKAN ERROR: Failed to create Win32 window surface!");
    }
  }

  void GraphicsDevice_Vulkan::pickPhysicalDevice()
  {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
      throw std::runtime_error("VULKAN ERROR: Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

    // Check if available devices are suitable
    for (const auto& device : devices) {
      if (isDeviceSuitable(device)) {
        m_PhysicalDevice = device;
        break;
      }
    }

    if (m_PhysicalDevice == VK_NULL_HANDLE) { // no suitable device found
      throw std::runtime_error("VULKAN ERROR: Failed to find a suitable GPU!");
    }
  }

  void GraphicsDevice_Vulkan::createLogicalDevice()
  {
    QueueFamilyIndices indices = findQueueFamilies(m_PhysicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    // Queue specification
    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
      VkDeviceQueueCreateInfo queueCreateInfo{};
      queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.queueFamilyIndex = queueFamily;
      queueCreateInfo.queueCount = 1;
      queueCreateInfo.pQueuePriorities = &queuePriority;
      queueCreateInfos.push_back(queueCreateInfo);
    }

    // Device feature specification
    VkPhysicalDeviceFeatures deviceFeatures{};

    // Logical device creation
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(m_DeviceExtensions.size());
    createInfo.ppEnabledExtensionNames = m_DeviceExtensions.data();

    if (m_EnableValidationLayers) {
      createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
      createInfo.ppEnabledLayerNames = m_ValidationLayers.data();
    }
    else {
      createInfo.enabledLayerCount = 0;
    }

    // Instantiate the logical device
    if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS) {
      throw std::runtime_error("VULKAN ERROR: Failed to create logical device!");
    }

    vkGetDeviceQueue(m_Device, indices.graphicsFamily.value(), 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_Device, indices.presentFamily.value(), 0, &m_PresentQueue);
  }

  void GraphicsDevice_Vulkan::createCommandPool() {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_PhysicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS) {
      throw std::runtime_error("VULKAN ERROR: Failed to create command pool!");
    }
  }

  bool GraphicsDevice_Vulkan::checkValidationLayerSupport()
  {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : m_ValidationLayers) {
      bool layerFound = false;

      for (const auto& layerProperties : availableLayers) {
        if (strcmp(layerName, layerProperties.layerName) == 0) {
          layerFound = true;
          break;
        }
      }

      if (!layerFound) {
        return false;
      }
    }

    return true;
  }

  void GraphicsDevice_Vulkan::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
  {
    // Buffer creation
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_Device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
      throw std::runtime_error("VULKAN ERROR: Failed to create buffer!");
    }

    // Memory requirements
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);

    // Memory allocation
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
      throw std::runtime_error("VUlKAN ERROR: Failed to allocate buffer memory!");
    }

    vkBindBufferMemory(m_Device, buffer, bufferMemory, 0);
  }

  void GraphicsDevice_Vulkan::createBuffer(GraphicsDevice_Vulkan& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
  {
    // Buffer creation
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device.getDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
      throw std::runtime_error("VULKAN ERROR: Failed to create buffer!");
    }

    // Memory requirements
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device.getDevice(), buffer, &memRequirements);

    // Memory allocation
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = device.findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device.getDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
      throw std::runtime_error("VUlKAN ERROR: Failed to allocate buffer memory!");
    }

    vkBindBufferMemory(device.getDevice(), buffer, bufferMemory, 0);
  }

  void GraphicsDevice_Vulkan::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
  {
    // TODO: Create separate command pool for optimization of short lived objects
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_CommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_Device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

      VkBufferCopy copyRegion{};
      copyRegion.srcOffset = 0; // optional
      copyRegion.dstOffset = 0; // optional
      copyRegion.size = size;
      vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_GraphicsQueue);

    // Cleanup
    vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &commandBuffer);
  }

  void GraphicsDevice_Vulkan::copyBuffer(GraphicsDevice_Vulkan& device, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
  {
    // TODO: Create separate command pool for optimization of short lived objects
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = device.m_CommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device.getDevice(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0; // optional
    copyRegion.dstOffset = 0; // optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(device.m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(device.m_GraphicsQueue);

    // Cleanup
    vkFreeCommandBuffers(device.getDevice(), device.m_CommandPool, 1, &commandBuffer);
  }

  std::vector<const char*> GraphicsDevice_Vulkan::getRequiredExtensions()
  {
    // Retrieve required Vulkan extensions
    uint32_t extensionCount = 0;
    std::vector<VkExtensionProperties> extensionProperties;

    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL); // get extension count
    extensionProperties.resize(extensionCount);
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensionProperties.data()); // fill out extensions

    m_RequiredExtensions.clear();

    // Required Win32 instance extensions
    bool win32SurfaceExtension = false;
    bool vkKHRSurfaceExtension = false;

    std::cout << "Available Vulkan extensions (* = required):\n";
    for (uint32_t i = 0; i < extensionCount; i++) {
      std::string extensionName = extensionProperties[i].extensionName;
      std::cout << '\t' << extensionName;

      // TODO: WIN32 SPECIFIC, needs to be fixed
      if (extensionName == "VK_KHR_win32_surface") {
        win32SurfaceExtension = true;
        m_RequiredExtensions.push_back(extensionName);
        std::cout << '*';
      }
      else if (extensionName == "VK_KHR_surface") {
        vkKHRSurfaceExtension = true;
        m_RequiredExtensions.push_back(extensionName);
        std::cout << '*';
      }

      std::cout << '\n';
    }

    // TODO: Refactor cross-platform extension requirements
    if (!win32SurfaceExtension || !vkKHRSurfaceExtension) {
      throw std::runtime_error("VULKAN ERROR: Required extensions not available!");
    }

    // Clear and resize extensionPointers to match requiredExtensions
    m_ExtensionPointers.resize(m_RequiredExtensions.size());
    for (size_t i = 0; i < m_RequiredExtensions.size(); i++) {
      m_ExtensionPointers[i] = m_RequiredExtensions[i].c_str();
    }

    std::vector<const char*> extensions(m_ExtensionPointers.data(), m_ExtensionPointers.data() + m_ExtensionPointers.size());
    if (m_EnableValidationLayers) {
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
  }

  VkResult GraphicsDevice_Vulkan::createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
  {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
      return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
      return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
  }

  void GraphicsDevice_Vulkan::destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
  {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
      func(instance, debugMessenger, pAllocator);
    }
  }

  void GraphicsDevice_Vulkan::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
  {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
  }

  bool GraphicsDevice_Vulkan::isDeviceSuitable(VkPhysicalDevice device)
  {
    // Acquire GPU properties
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    // Acquire GPU features
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    // TODO: For now, we just pick any GPU, in the future we should evaluate which one(s) in a more dedicated way
    QueueFamilyIndices indices = findQueueFamilies(device);
    bool extensionsSupported = checkDeviceExtensionSupport(device);
    bool swapChainAdequate = false;
    if (extensionsSupported) {
      SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
      swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return indices.isComplete() && extensionsSupported && swapChainAdequate;
  }

  QueueFamilyIndices GraphicsDevice_Vulkan::findQueueFamilies(VkPhysicalDevice device)
  {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
      if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        indices.graphicsFamily = i;
      }

      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);

      if (presentSupport) {
        indices.presentFamily = i;
      }

      if (indices.isComplete()) {
        break;
      }

      i++;
    }

    return indices;
  }

  bool GraphicsDevice_Vulkan::checkDeviceExtensionSupport(VkPhysicalDevice device)
  {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(m_DeviceExtensions.begin(), m_DeviceExtensions.end());

    for (const auto& extension : availableExtensions) {
      requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
  }

  SwapChainSupportDetails GraphicsDevice_Vulkan::querySwapChainSupport(VkPhysicalDevice device)
  {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);

    if (formatCount != 0) {
      details.formats.resize(formatCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
      details.presentModes.resize(presentModeCount);
      vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, details.presentModes.data());
    }

    return details;
  }

  uint32_t GraphicsDevice_Vulkan::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
  {
    // Query available memory types
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
      if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
        return i;
      }
    }

    throw std::runtime_error("VULKAN ERROR: Failed to find a suitable memory type!");
  }

  VKAPI_ATTR VkBool32 VKAPI_CALL GraphicsDevice_Vulkan::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
     std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
     return VK_FALSE;
  }

  // ------ General ------

}
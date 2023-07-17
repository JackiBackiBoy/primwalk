#ifndef FZ_GRAPHICS_DEVICE_HEADER
#define FZ_GRAPHICS_DEVICE_HEADER

// vendor
#ifdef FZ_WIN32
  #define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

// FZUI
#include "fzui/core.hpp"
#include "fzui/window.hpp"

// std
#include <array>
#include <atomic>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace fz {
  // ------ Vulkan ------
  struct UniformBufferObject {
    alignas(16) glm::mat4 proj;
  };

  struct FZ_API SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  };

  struct FZ_API QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
      return graphicsFamily.has_value() && presentFamily.has_value();
    }
  };

  class FZ_API GraphicsDevice_Vulkan {
    public:
      GraphicsDevice_Vulkan(Window& window);
      ~GraphicsDevice_Vulkan();

      // Forbid copy and move semantics
      GraphicsDevice_Vulkan(const GraphicsDevice_Vulkan&) = delete;
      GraphicsDevice_Vulkan& operator=(const GraphicsDevice_Vulkan&) = delete;
      GraphicsDevice_Vulkan(GraphicsDevice_Vulkan&&) = delete;
      GraphicsDevice_Vulkan& operator=(GraphicsDevice_Vulkan&&) = delete;

      void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory);
      void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
      void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
      VkCommandBuffer beginSingleTimeCommands();
      void endSingleTimeCommands(VkCommandBuffer commandBuffer);
      uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

      // Getters
      VkDevice getDevice();
      VkPhysicalDevice getPhysicalDevice();
      VkCommandPool getCommandPool();
      VkSurfaceKHR getSurface();
      VkQueue getGraphicsQueue();
      VkQueue getPresentQueue();
      SwapChainSupportDetails getSwapChainSupport();
      QueueFamilyIndices findPhysicalQueueFamilies();

    private:
      void createInstance();
      void setupDebugMessenger();
      void createSurface();
      void pickPhysicalDevice();
      void createLogicalDevice();
      void createCommandPool();
      
      bool checkValidationLayerSupport();
      std::vector<const char*> getRequiredExtensions();
      VkResult createDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger);
      void destroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator);
      void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
      bool isDeviceSuitable(VkPhysicalDevice device);
      QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
      bool checkDeviceExtensionSupport(VkPhysicalDevice device);
      SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

      Window& m_Window;
      uint32_t m_CurrentFrame = 0;
      VkInstance m_Instance = VK_NULL_HANDLE;
      VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
      VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
      VkDevice m_Device = VK_NULL_HANDLE;
      VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
      VkQueue m_PresentQueue = VK_NULL_HANDLE;
      VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
      VkCommandPool m_CommandPool = VK_NULL_HANDLE;
      VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
      
      std::vector<std::string> m_RequiredExtensions;
      std::vector<const char*> m_ExtensionPointers;
      std::vector<VkBuffer> m_UniformBuffers;
      std::vector<VkDeviceMemory> m_UniformBuffersMemory;
      std::vector<void*> m_UniformBuffersMapped;
      bool m_BindlessSupported = false;
      

      static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
      static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

      const std::vector<const char*> m_DeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
      };

      const std::vector<const char*> m_ValidationLayers = {
        "VK_LAYER_KHRONOS_validation"
      };

#ifdef NDEBUG
      const bool m_EnableValidationLayers = false;
#else
      const bool m_EnableValidationLayers = true;
#endif
  };

  // ------ General ------
  class FZ_API GraphicsDevice {
    public:
      GraphicsDevice() {};
      ~GraphicsDevice() {};

      std::shared_ptr<void> m_InternalState;
  };

  // Global device instance helper
  inline GraphicsDevice_Vulkan*& GetDevice() {
    static GraphicsDevice_Vulkan* device = nullptr;
    return device;
  }
}

#endif
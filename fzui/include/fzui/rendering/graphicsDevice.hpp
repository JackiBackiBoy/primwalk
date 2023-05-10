#ifndef FZ_GRAPHICS_DEVICE_HEADER
#define FZ_GRAPHICS_DEVICE_HEADER

// vendor
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

// FZUI
#include "fzui/core.hpp"
#include "fzui/rendering/graphicsPipeline.hpp"

// std
#include <array>
#include <atomic>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <windows.h>

namespace fz {
  // ------ DirectX 12 ------
  class FZ_API GraphicsDevice_DX12 {
    public:
      GraphicsDevice_DX12() {};
      ~GraphicsDevice_DX12() {};
  };

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
      GraphicsDevice_Vulkan(HWND hWnd);
      ~GraphicsDevice_Vulkan();

      // Getters
      VkDevice getDevice();
      VkCommandPool getCommandPool();
      VkSurfaceKHR getSurface();
      VkQueue getGraphicsQueue();
      VkQueue getPresentQueue();
      SwapChainSupportDetails getSwapChainSupport();
      QueueFamilyIndices findPhysicalQueueFamilies();
      static void createBuffer(GraphicsDevice_Vulkan& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory);
      static void copyBuffer(GraphicsDevice_Vulkan& device, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    private:
      void createInstance();
      void setupDebugMessenger();
      void createSurface();
      void pickPhysicalDevice();
      void createLogicalDevice();
      void createCommandPool();
      
      void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory);
      void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
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
      uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

      HWND m_Handle = NULL;
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
}

#endif
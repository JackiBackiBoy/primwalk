#ifndef FZ_GRAPHICS_DEVICE_HEADER
#define FZ_GRAPHICS_DEVICE_HEADER

// vendor
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

// FZUI
#include "fzui/core.hpp"
#include "fzui/rendering/graphicsPipeline.hpp"

// std
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

      void drawFrame(GraphicsPipeline& gPipeline);

      // Getters
      VkDevice getDevice();
      VkRenderPass getRenderPass();

    private:
      void createInstance();
      void setupDebugMessenger();
      void createSurface();
      void pickPhysicalDevice();
      void createLogicalDevice();
      void createSwapChain();
      void createImageViews();
      void createRenderPass();
      void createFrameBuffers();
      void createCommandPool();
      void createCommandBuffer();
      void createSyncObjects();
      void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
      void endRecordCommandBuffer(VkCommandBuffer commandBuffer);
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
      VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
      VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
      VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

      HWND m_Handle = NULL;
      VkInstance m_Instance = VK_NULL_HANDLE;
      VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
      VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
      VkDevice m_Device = VK_NULL_HANDLE;
      VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
      VkQueue m_PresentQueue = VK_NULL_HANDLE;
      VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
      VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
      VkFormat m_SwapChainImageFormat;
      VkExtent2D m_SwapChainExtent;
      VkRenderPass m_RenderPass = VK_NULL_HANDLE;
      VkCommandPool m_CommandPool = VK_NULL_HANDLE;
      VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;
      VkSemaphore m_ImageAvailableSemaphore;
      VkSemaphore m_RenderFinishedSemaphore;
      VkFence m_InFlightFence;
      std::vector<std::string> m_RequiredExtensions;
      std::vector<const char*> m_ExtensionPointers;
      std::vector<VkImage> m_SwapChainImages;
      std::vector<VkImageView> m_SwapChainImageViews;
      std::vector<VkFramebuffer> m_SwapChainFramebuffers;

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
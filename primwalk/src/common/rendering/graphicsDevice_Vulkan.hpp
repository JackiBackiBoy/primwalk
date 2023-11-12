#pragma once

// vendor
#ifdef PW_WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

// primwalk
#include "../../core.hpp"
#include "../../window.hpp"
#include "graphicsDevice.hpp"
#include "image.hpp"
#include "descriptors.hpp"

// std
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef PW_WIN32
#include <windows.h>
#endif

#ifdef PW_MACOS
typedef void* id;
typedef VkFlags VkMetalSurfaceCreateFlagsEXT;
typedef struct VkMetalSurfaceCreateInfoEXT
{
  VkStructureType                 sType;
  const void* pNext;
  VkMetalSurfaceCreateFlagsEXT    flags;
  const void* pLayer;
} VkMetalSurfaceCreateInfoEXT;

typedef VkResult(*PFN_vkCreateMetalSurfaceEXT)(VkInstance, const VkMetalSurfaceCreateInfoEXT*, const VkAllocationCallbacks*, VkSurfaceKHR*);
#endif

namespace pw {
	struct CommandList_Vulkan {
		VkCommandBuffer commandList = VK_NULL_HANDLE;
	};

	struct PW_API SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities{};
		std::vector<VkSurfaceFormatKHR> formats{};
		std::vector<VkPresentModeKHR> presentModes{};
	};

	struct PW_API QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		inline bool isComplete() const {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	class PW_API GraphicsDevice_Vulkan : public GraphicsDevice {
	public:
		GraphicsDevice_Vulkan(Window& window);
		~GraphicsDevice_Vulkan();

		// Forbid copy and move semantics
		GraphicsDevice_Vulkan(const GraphicsDevice_Vulkan&) = delete;
		GraphicsDevice_Vulkan& operator=(const GraphicsDevice_Vulkan&) = delete;
		GraphicsDevice_Vulkan(GraphicsDevice_Vulkan&&) = delete;
		GraphicsDevice_Vulkan& operator=(GraphicsDevice_Vulkan&&) = delete;

		CommandList beginFrame() override;
		void endFrame() override;
		void waitForGPU() override;

		void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
			VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void endSingleTimeCommands(VkCommandBuffer commandBuffer);
		VkCommandBuffer beginSingleTimeCommands();
		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
		VkFormat getSupportedDepthFormat();

		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		// Getters
		inline VkDevice getDevice() const { return m_Device; }
		inline VkPhysicalDevice getPhysicalDevice() const { return m_PhysicalDevice; }
		inline VkCommandPool getCommandPool() const { return m_CommandPool; }
		inline VkSurfaceKHR getSurface() const { return m_Surface; }
		inline VkQueue getGraphicsQueue() const { return m_GraphicsQueue; }
		inline VkQueue getPresentQueue() const { return m_PresentQueue; }
		inline SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(m_PhysicalDevice); }
		inline QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(m_PhysicalDevice); }
		inline DescriptorPool& getBindlessPool() { return *m_BindlessDescriptorPool; }

		static constexpr size_t MAX_FRAMES_IN_FLIGHT = 2;
		static constexpr uint32_t MAX_IMAGE_DESCRIPTORS = 4096;
		static constexpr uint32_t MAX_UBO_DESCRIPTORS = 32;
		static constexpr uint32_t MAX_SSBO_DESCRIPTORS = 32;

	private:
		void createInstance();
		void setupDebugMessenger();
		void createSurface();
		void pickPhysicalDevice();
		void createLogicalDevice();
		void createCommandPool();
		void createDescriptorPool();
		static std::vector<std::string> getRequiredVulkanInstanceExtensions();

		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void destroyDebugUtilsMessengerEXT(
			VkInstance instance,
			VkDebugUtilsMessengerEXT debugMessenger,
			const VkAllocationCallbacks* pAllocator);

		bool checkValidationLayerSupport() const;
		bool checkDeviceExtensionSupport(VkPhysicalDevice device) const;
		bool isDeviceSuitable(VkPhysicalDevice device);

		VkResult createDebugUtilsMessengerEXT(
			VkInstance instance,
			const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkDebugUtilsMessengerEXT* pDebugMessenger);
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
		std::vector<const char*> getInstanceExtensions();

		VkInstance m_Instance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_Device = VK_NULL_HANDLE;
		VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
		VkQueue m_PresentQueue = VK_NULL_HANDLE;
		VkCommandPool m_CommandPool = VK_NULL_HANDLE;
		std::unique_ptr<DescriptorPool> m_BindlessDescriptorPool{};
		
		Window& m_Window;

		std::vector<const char*> m_DesiredInstanceExtensions = {
			#ifdef _DEBUG
				VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
			#endif
			VK_KHR_SURFACE_EXTENSION_NAME,
		};
		std::vector<std::string> m_RequiredExtensions;
		std::vector<const char*> m_ExtensionPointers;
		bool m_BindlessSupported = false;

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);

		const std::vector<const char*> m_DeviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
		};

		const std::vector<const char*> m_ValidationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

		//std::unordered_map<Image*, uint32_t> m_TextureIDs{};
		//std::set<uint32_t> m_VacantTextureIDs{};

		#ifdef NDEBUG
		const bool m_EnableValidationLayers = false;
		#else
		const bool m_EnableValidationLayers = true;
		#endif
	};
}
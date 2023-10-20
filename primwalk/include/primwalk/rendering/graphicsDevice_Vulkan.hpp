#pragma once

// vendor
#ifdef PW_WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/window.hpp"
#include "primwalk/rendering/graphicsDevice.hpp"
#include "primwalk/rendering/image.hpp"
#include "primwalk/rendering/descriptors.hpp"

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

		void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
			VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void endSingleTimeCommands(VkCommandBuffer commandBuffer);
		VkCommandBuffer beginSingleTimeCommands();
		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
		VkFormat getSupportedDepthFormat();

		// Getters
		inline VkDevice getDevice() const { return m_Device; }
		inline VkPhysicalDevice getPhysicalDevice() const { return m_PhysicalDevice; }
		inline VkCommandPool getCommandPool() const { return m_CommandPool; }
		inline VkSurfaceKHR getSurface() const { return m_Surface; }
		inline VkQueue getGraphicsQueue() const { return m_GraphicsQueue; }
		inline VkQueue getPresentQueue() const { return m_PresentQueue; }
		inline SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(m_PhysicalDevice); }
		inline QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(m_PhysicalDevice); }

		std::unique_ptr<DescriptorSetLayout> m_TextureSetLayout{};
		std::unique_ptr<DescriptorPool> m_BindlessDescriptorPool{};
		VkDescriptorSet m_TextureDescriptorSet = VK_NULL_HANDLE;

		bool getTextureID(Image* image, uint32_t* id); // Returns false if no texture ID is associated with the image
		uint32_t addTextureID(Image* image);
		void removeTextureID(Image* image);

		static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
		static constexpr int MAX_IMAGE_DESCRIPTORS = 1024;

	private:
		void createInstance();
		void setupDebugMessenger();
		void createSurface();
		void pickPhysicalDevice();
		void createLogicalDevice();
		void createCommandPool();
		void createDescriptorPool();
		void createDescriptorSetLayouts();
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
		std::vector<const char*> getRequiredExtensions();

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

		std::unordered_map<Image*, uint32_t> m_TextureIDs{};
		std::set<uint32_t> m_VacantTextureIDs{};

		#ifdef NDEBUG
		const bool m_EnableValidationLayers = false;
		#else
		const bool m_EnableValidationLayers = true;
		#endif
	};
}
#ifndef FZ_WINDOW_BASE_HEADER
#define FZ_WINDOW_BASE_HEADER

// std
#include <atomic>
#include <memory>
#include <mutex>
#include <vector>
#include <condition_variable>

// FZUI
#include "fzui/core.hpp"

// Vendor
#include <vulkan/vulkan.h>
typedef void* id;

typedef VkFlags VkMetalSurfaceCreateFlagsEXT;

typedef struct VkMetalSurfaceCreateInfoEXT
{
    VkStructureType                 sType;
    const void*                     pNext;
    VkMetalSurfaceCreateFlagsEXT    flags;
    const void*                     pLayer;
} VkMetalSurfaceCreateInfoEXT;

typedef VkResult (*PFN_vkCreateMetalSurfaceEXT)(VkInstance, const VkMetalSurfaceCreateInfoEXT*, const VkAllocationCallbacks*, VkSurfaceKHR*);

namespace fz {
  class FZ_API GraphicsDevice_Vulkan;
  class FZ_API WindowBase {
    public:
      WindowBase() {}
      virtual ~WindowBase() {}

      virtual std::vector<std::string> getRequiredVulkanInstanceExtensions() = 0;
      virtual VkResult createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) = 0;

      // Event functions
      virtual void onUpdate(float dt) {};
      virtual void onRender(float dt) {};

      // Getters
      virtual int getWidth() const = 0;
      virtual int getHeight() const = 0;

    protected:
      std::atomic<bool> m_IsMinimized = false;
      std::mutex m_RenderingMutex;
      std::condition_variable m_RenderingCondition;
      std::shared_ptr<GraphicsDevice_Vulkan> m_GraphicsDevice;
  };
}

// Platform specific implementation
#if defined(FZ_WIN32)
  #include "fzui/windows/core/window_win32.hpp"
  namespace fz {
    typedef WindowWin32 Window;
  }
#elif defined(FZ_MACOS)
  #include "fzui/macos/core/window_osx.hpp"
  namespace fz {
    typedef WindowOSX Window;
  }
#endif

#endif

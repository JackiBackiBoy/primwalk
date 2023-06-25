#ifndef FZ_WINDOW_BASE_HEADER
#define FZ_WINDOW_BASE_HEADER

#ifdef FZ_WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

// vendor
#include <vulkan/vulkan.h>

// std
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

// FZUI
#include "fzui/core.hpp"
#include "fzui/color.hpp"
#include "fzui/ui/uiElement.hpp"
#include "fzui/ui/uiEvent.hpp"
#include "fzui/ui/uiButton.hpp"
#include "fzui/rendering/systems/uiRenderSystem.hpp"

#ifdef FZ_MACOS
typedef void* id;
typedef VkFlags VkMetalSurfaceCreateFlagsEXT;
typedef struct VkMetalSurfaceCreateInfoEXT
{
  VkStructureType                 sType;
  const void* pNext;
  VkMetalSurfaceCreateFlagsEXT    flags;
  const void* pLayer;
} VkMetalSurfaceCreateInfoEXT;

typedef VkResult (*PFN_vkCreateMetalSurfaceEXT)(VkInstance, const VkMetalSurfaceCreateInfoEXT*, const VkAllocationCallbacks*, VkSurfaceKHR*);
#endif

namespace fz {
  // Forward declarations
  class FZ_API GraphicsDevice_Vulkan;
  class FZ_API Renderer;

  class FZ_API WindowBase {
    public:
      WindowBase(const std::string& name, int width, int height) :
        m_Name(name), m_Width(width), m_Height(height) {};
      virtual ~WindowBase() {}

      virtual std::vector<std::string> getRequiredVulkanInstanceExtensions() = 0;
      virtual VkResult createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) = 0;
      virtual bool isCursorInTitleBar(int x, int y) = 0;
      virtual bool isCursorOnBorder(int x, int y) = 0;

      // Event functions
      virtual void onUpdate(float dt) {};

      virtual void processEvent(const UIEvent& event) {
        m_UIRenderSystem->processEvent(event);
      }

      template<typename UIType, typename Key, typename... Args>
      UIType& makeElement(Key const& key, Args&&...args) {
        auto tmp = std::make_unique<UIType>(std::forward<Args>(args)...);
        auto& ref = *tmp;

        m_UIRenderSystem->submitElement(std::move(tmp));
        return ref;
      }

      // Getters
      virtual int getWidth() const = 0;
      virtual int getHeight() const = 0;

      // Setters
      inline void setBackgroundColor(Color color) { m_BackgroundColor = color; }
      virtual void setMinimumSize(uint32_t width, uint32_t height) = 0;

    protected:
      std::string m_Name;
      int m_Width;
      int m_Height;
      uint32_t m_MinWidth = 200;
      uint32_t m_MinHeight = 200;
      Color m_BackgroundColor;

      std::shared_ptr<GraphicsDevice_Vulkan> m_GraphicsDevice{};
      std::shared_ptr<Renderer> m_Renderer{};
      std::unique_ptr<UIRenderSystem> m_UIRenderSystem{};
      std::atomic<bool> m_IsMinimized = false;
      std::mutex m_RenderingMutex;
      std::condition_variable m_RenderingCondition;
  };
}

// Platform specific implementation
#if defined(FZ_WIN32)
  #include "fzui/windows/core/window_win32.hpp"
#elif defined(FZ_MACOS)
  #include "fzui/macos/core/window_osx.hpp"
#endif
#endif

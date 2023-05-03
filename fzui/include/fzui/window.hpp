#ifndef FZ_WINDOW_BASE_HEADER
#define FZ_WINDOW_BASE_HEADER

// std
#include <atomic>
#include <memory>
#include <mutex>
#include <condition_variable>

// FZUI
#include "fzui/core.hpp"
#include "fzui/data/texture.hpp"
#include "fzui/rendering/graphicsDevice.hpp"
#include "fzui/rendering/graphicsPipeline.hpp"

namespace fz {
  class FZ_API WindowBase;

  class FZ_API WindowBase {
    public:
      WindowBase() {}
      virtual ~WindowBase() {}

      // Event functions
      virtual void onUpdate(float dt) {};
      virtual void onRender(float dt) {};

      // Getters
      virtual int getWidth() const = 0;
      virtual int getHeight() const = 0;

    protected:
      Texture m_WindowIcon;
      std::atomic<bool> m_IsMinimized = false;
      std::mutex m_RenderingMutex;
      std::condition_variable m_RenderingCondition;
      GraphicsPipeline* m_GraphicsPipeline = nullptr;
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

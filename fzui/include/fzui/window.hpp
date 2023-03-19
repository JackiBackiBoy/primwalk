#ifndef FZ_WINDOW_BASE_HEADER
#define FZ_WINDOW_BASE_HEADER

#include "fzui/core.hpp"
#include "fzui/windows/data/texture.hpp"

namespace fz {
  class FZ_API WindowBase;

  class FZ_API WindowBase {
    public:
      WindowBase() {}
      virtual ~WindowBase() {}

      // Event functions
      virtual void onUpdate(const float& dt) {};
      virtual void onRender(const float& dt) {};

      // Getters
      virtual int getWidth() const = 0;
      virtual int getHeight() const = 0;

    protected:
      Texture m_WindowIcon;
  };
}

// Platform specific implementation
#if defined(FZ_WIN32)
  #include "fzui/windows/core/window_win32.hpp"
  namespace fz {
    typedef WindowWin32 Window;
  }
#elif defined(FZ_MACOS)
#endif

#endif

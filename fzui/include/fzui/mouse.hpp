#ifndef FZ_MOUSE_BASE_HEADER
#define FZ_MOUSE_BASE_HEADER

// FZUI
#include "fzui/core.hpp"

// vendor
#include <glm/glm.hpp>


namespace fz {
  class FZ_API MouseBase;

  class FZ_API MouseBase {
    public:
      virtual ~MouseBase() {}

      // Function
      virtual glm::vec2 getRelativePos() = 0;
      virtual glm::vec2 getAbsolutePos() = 0;

    protected:
      glm::vec2 m_RelativePos = { 0, 0 };
  };
}

// Platform specific implementation
#if defined(FZ_WIN32)
  #include "fzui/windows/input/mouse_win32.hpp"
  namespace fz {
    typedef MouseWin32 Mouse;
  }
#elif defined(FZ_MACOS)
#endif

#endif
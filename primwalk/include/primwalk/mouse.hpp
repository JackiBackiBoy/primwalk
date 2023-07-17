#ifndef PW_MOUSE_BASE_HEADER
#define PW_MOUSE_BASE_HEADER

// primwalk
#include "primwalk/core.hpp"

// vendor
#include <glm/glm.hpp>


namespace pw {
  class PW_API MouseBase;

  class PW_API MouseBase {
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
#if defined(PW_WIN32)
  #include "primwalk/windows/input/mouse_win32.hpp"
  namespace pw {
    typedef MouseWin32 Mouse;
  }
#elif defined(PW_MACOS)
  #include "primwalk/macos/input/mouse_osx.hpp"
  namespace fz {
    typedef MouseOSX Mouse;
  }
#endif

#endif

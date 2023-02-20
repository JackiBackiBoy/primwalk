#ifndef FZ_WINDOW_BASE_HEADER
#define FZ_WINDOW_BASE_HEADER

#include "fzui/core.hpp"

// Platform specific implementations
#include "fzui/windows/core/window_win32.hpp"


namespace fz {
  class FZ_API WindowBase {
    public:
      WindowBase() {}
      virtual ~WindowBase() {}
  };

  typedef WindowWin32 Window;
}
#endif

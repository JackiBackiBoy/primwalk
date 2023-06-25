#ifndef FZ_MOUSE_BUTTONS_HEADER
#define FZ_MOUSE_BUTTONS_HEADER

// FZUI
#include "fzui/core.hpp"

namespace fz {
  struct FZ_API MouseButtons {
    bool leftButton = false;
    bool middleButton = false;
    bool rightButton = false;
    // TODO: Add more mouse button states
  };
}
#endif
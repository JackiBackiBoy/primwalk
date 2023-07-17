#ifndef PW_MOUSE_BUTTONS_HEADER
#define PW_MOUSE_BUTTONS_HEADER

// primwalk
#include "primwalk/core.hpp"

namespace pw {
  struct PW_API MouseButtons {
    bool leftButton = false;
    bool middleButton = false;
    bool rightButton = false;
    // TODO: Add more mouse button states
  };
}
#endif
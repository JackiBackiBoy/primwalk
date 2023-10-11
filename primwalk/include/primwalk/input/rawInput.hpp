#pragma once

// primwalk
#include "primwalk/input/input.hpp"

namespace pw::input::rawinput {
  void parseMessage(void* lparam);
  void getKeyboardState(pw::input::KeyboardState* state);
  void getMouseState(pw::input::MouseState* state);
  void update();
}
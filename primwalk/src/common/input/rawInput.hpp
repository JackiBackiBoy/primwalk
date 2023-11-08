#pragma once

// primwalk
#include "input.hpp"

namespace pw::input::rawinput {
	void initialize();
	void parseMessage(void* lparam);
	void getKeyboardState(pw::input::KeyboardState* state);
	void getMouseState(pw::input::MouseState* state);
	void update();
}
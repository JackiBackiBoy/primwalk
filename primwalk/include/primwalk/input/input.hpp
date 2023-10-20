#pragma once

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/input/keycode.hpp"

// vendor
#include <glm/glm.hpp>

namespace pw::input {
	struct PW_API KeyboardState {
		bool buttons[256] = {};
	};

	struct PW_API MouseState {
		glm::vec2 position = { 0, 0 };
		glm::vec2 deltaPosition = { 0, 0 };
		float wheelDelta = 0.0f;
		bool leftDown = false;
		bool rightDown = false;
		bool middleDown = false;
	};

	void initialize();
	void PW_API getKeyboardState(pw::input::KeyboardState* state);
	void PW_API getMouseState(pw::input::MouseState* state);
	void update();

	bool PW_API isDown(const KeyCode& button);
}
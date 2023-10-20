#include "primwalk/input/rawInput.hpp"

// windows
#include <windows.h>

// std
#include <iostream>
#include <array>
#include <vector>
#include <mutex>

namespace pw::input::rawinput {
	static pw::input::MouseState mouse{};
	static pw::input::KeyboardState keyboard{};
	static std::vector<RAWINPUT*> messages{};
	static std::mutex inputMutex{};

	void parseRawInput(const RAWINPUT& raw) {
		if (raw.header.dwType == RIM_TYPEKEYBOARD)
		{
			const RAWKEYBOARD& rawkeyboard = raw.data.keyboard;
			if (rawkeyboard.VKey < 256)
			{
				if (rawkeyboard.Flags == RI_KEY_MAKE)
				{
					// key down
					keyboard.buttons[rawkeyboard.VKey] = true;
				}
			}
			else
			{
				assert(0);
			}
		}
		else if (raw.header.dwType == RIM_TYPEMOUSE) {
			mouse.deltaPosition.x = raw.data.mouse.lLastX;
			mouse.deltaPosition.y = raw.data.mouse.lLastY;

			if (raw.data.mouse.usButtonFlags == RI_MOUSE_WHEEL) {
				mouse.wheelDelta += float((SHORT)raw.data.mouse.usButtonData) / float(WHEEL_DELTA);
			}
		}
	}

	void parseMessage(void* lparam) {
		inputMutex.lock();

		UINT size{};
		UINT result{};

		result = GetRawInputData((HRAWINPUT)lparam, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
		assert(result == 0);

		RAWINPUT input[sizeof(RAWINPUT)];
		result = GetRawInputData((HRAWINPUT)lparam, RID_INPUT, &input, &size, sizeof(RAWINPUTHEADER));

		if (result == size) {
			messages.push_back(input);
		}

		inputMutex.unlock();
	}

	void getKeyboardState(pw::input::KeyboardState* state) {
		*state = keyboard;
	}

	void getMouseState(pw::input::MouseState* state) {
		*state = mouse;
	}

	void update() {
		inputMutex.lock();

		mouse.deltaPosition = { 0, 0 };
		mouse.wheelDelta = 0.0f;

		for (auto& input : messages) {
			parseRawInput(*(PRAWINPUT)input);
		}

		messages.clear();

		inputMutex.unlock();
	}

}
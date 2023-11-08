#include "rawInput.hpp"

// windows
#include <windows.h>

// std
#include <iostream>
#include <array>
#include <vector>

namespace pw::input::rawinput {
	static pw::input::MouseState mouse{};
	static pw::input::KeyboardState keyboard{};
	static std::vector<RAWINPUT*> messages{};

	void initialize() {
		// Raw input device registration
		RAWINPUTDEVICE rid[2] = {};

		// Register mouse:
		rid[0].usUsagePage = 0x01;
		rid[0].usUsage = 0x02;
		rid[0].dwFlags = 0;
		rid[0].hwndTarget = 0;

		// Register keyboard:
		rid[1].usUsagePage = 0x01;
		rid[1].usUsage = 0x06;
		rid[1].dwFlags = 0;
		rid[1].hwndTarget = 0;

		RegisterRawInputDevices(rid, 2, sizeof(rid[0]));
	}

	void parseRawInput(RAWINPUT* raw) {
		if (raw->header.dwType == RIM_TYPEKEYBOARD)
		{
			const RAWKEYBOARD& rawkeyboard = raw->data.keyboard;
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
		else if (raw->header.dwType == RIM_TYPEMOUSE) {
			mouse.deltaPosition.x = raw->data.mouse.lLastX;
			mouse.deltaPosition.y = raw->data.mouse.lLastY;

			if (raw->data.mouse.usButtonFlags == RI_MOUSE_WHEEL) {
				mouse.wheelDelta += float((SHORT)raw->data.mouse.usButtonData) / float(WHEEL_DELTA);
			}
		}
	}

	void parseMessage(void* lparam) {
		UINT size = 0;
		UINT result = 0;

		result = GetRawInputData((HRAWINPUT)lparam, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
		assert(result == 0);

		RAWINPUT* input = (RAWINPUT*)malloc(size);
		result = GetRawInputData((HRAWINPUT)lparam, RID_INPUT, input, &size, sizeof(RAWINPUTHEADER));

		if (result == size) {
			messages.push_back(input);
			return;
		}

		free(input);
	}

	void getKeyboardState(pw::input::KeyboardState* state) {
		*state = keyboard;
	}

	void getMouseState(pw::input::MouseState* state) {
		*state = mouse;
	}

	void update() {
		mouse.deltaPosition = { 0.0f, 0.0f };
		mouse.wheelDelta = 0.0f;

		for (auto input : messages) {
			parseRawInput(input);
			free(input);
		}

		messages.clear();
	}

}
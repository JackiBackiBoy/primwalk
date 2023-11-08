#pragma once

// std
#include <cstdint>

#ifdef WIN32
#include <windows.h>
#endif

namespace pw {
	enum class KeyModifier : uint8_t {
		None = 0x00,
		Shift = 0x01, ///< The shift key is being held.
		Control = 0x02, ///< The control key is being held.
		Alt = 0x04, ///< The alt-key, option-key or meta-key is being held.
		Super = 0x08, ///< The windows-key, key-key or super-key is being held.
	};

	constexpr KeyModifier operator|(KeyModifier const& lhs, KeyModifier const& rhs) noexcept
	{
		return static_cast<KeyModifier>(uint8_t(lhs) | uint8_t(rhs));
	}

	constexpr KeyModifier operator&(KeyModifier const& lhs, KeyModifier const& rhs) noexcept
	{
		return static_cast<KeyModifier>(uint8_t(lhs) & uint8_t(rhs));
	}

	constexpr KeyModifier& operator|=(KeyModifier& lhs, KeyModifier const& rhs) noexcept
	{
		return lhs = lhs | rhs;
	}

	enum class KeyCode : int {
		None = 0,

		MouseButtonLeft,
		MouseButtonRight,
		MouseButtonMiddle,

		KeyboardButtonUp,
		KeyboardButtonDown,
		KeyboardButtonLeft,
		KeyboardButtonRight,
		KeyboardButtonSpace,
		KeyboardButtonRShift,
		KeyboardButtonLShift,
		KeyboardButtonF1,
		KeyboardButtonF2,
		KeyboardButtonF3,
		KeyboardButtonF4,
		KeyboardButtonF5,
		KeyboardButtonF6,
		KeyboardButtonF7,
		KeyboardButtonF8,
		KeyboardButtonF9,
		KeyboardButtonF10,
		KeyboardButtonF11,
		KeyboardButtonF12,
		KeyboardButtonEnter,
		KeyboardButtonEscape,
		KeyboardButtonHome,
		KeyboardButtonRControl,
		KeyboardButtonLControl,
		KeyboardButtonDelete,
		KeyboardButtonBackspace,
		KeyboardButtonTab,
		KeyboardButtonPageDown,
		KeyboardButtonPageUp,
		KeyboardButtonNumpad0,
		KeyboardButtonNumpad1,
		KeyboardButtonNumpad2,
		KeyboardButtonNumpad3,
		KeyboardButtonNumpad4,
		KeyboardButtonNumpad5,
		KeyboardButtonNumpad6,
		KeyboardButtonNumpad7,
		KeyboardButtonNumpad8,
		KeyboardButtonNumpad9,
		KeyboardButtonMultiply,
		KeyboardButtonAdd,
		KeyboardButtonSeparator,
		KeyboardButtonSubtract,
		KeyboardButtonDecimal,
		KeyboardButtonDivide,
		KeyboardButtonPause,
		KeyboardButtonEnd,
		KeyboardButtonPrintScreen,
		KeyboardButtonInsert,
		KeyboardButtonAlpha0,
		KeyboardButtonAlpha1,
		KeyboardButtonAlpha2,
		KeyboardButtonAlpha3,
		KeyboardButtonAlpha4,
		KeyboardButtonAlpha5,
		KeyboardButtonAlpha6,
		KeyboardButtonAlpha7,
		KeyboardButtonAlpha8,
		KeyboardButtonAlpha9,

		KeyboardButtonA = 65, // letter A
		KeyboardButtonB = 66,
		KeyboardButtonC = 67,
		KeyboardButtonD = 68,
		KeyboardButtonE = 69,
		KeyboardButtonF = 70,
		KeyboardButtonG = 71,
		KeyboardButtonH = 72,
		KeyboardButtonI = 73,
		KeyboardButtonJ = 74,
		KeyboardButtonK = 75,
		KeyboardButtonL = 76,
		KeyboardButtonM = 77,
		KeyboardButtonN = 78,
		KeyboardButtonO = 79,
		KeyboardButtonP = 80,
		KeyboardButtonQ = 81,
		KeyboardButtonR = 82,
		KeyboardButtonS = 83,
		KeyboardButtonT = 84,
		KeyboardButtonU = 85,
		KeyboardButtonV = 86,
		KeyboardButtonW = 87,
		KeyboardButtonX = 88,
		KeyboardButtonY = 89,
		KeyboardButtonZ = 90,

		GamepadRangeStart = 256, // do not use!

		GamepadButtonUp,
		GamepadButtonLeft,
		GamepadButtonDown,
		GamepadButtonRight,
		GamepadButton1,
		GamepadButton2,
		GamepadButton3,
		GamepadButton4,
		GamepadButton5,
		GamepadButton6,
		GamepadButton7,
		GamepadButton8,
		GamepadButton9,
		GamepadButton10,
		GamepadButton11,
		GamepadButton12,
		GamepadButton13,
		GamepadButton14,
	};
}

namespace pw::input {
#ifdef WIN32
	inline static KeyCode toKeyCode(int keyCode) {
		switch (keyCode) {
		case VK_BACK:
			return KeyCode::KeyboardButtonBackspace;
		case VK_TAB:
			return KeyCode::KeyboardButtonTab;
		case VK_RETURN:
			return KeyCode::KeyboardButtonEnter;
		case VK_PAUSE:
			return KeyCode::KeyboardButtonPause;
		case VK_ESCAPE:
			return KeyCode::KeyboardButtonEscape;
		case VK_SPACE:
			return KeyCode::KeyboardButtonSpace;
		case VK_PRIOR:
			return KeyCode::KeyboardButtonPageUp;
		case VK_NEXT:
			return KeyCode::KeyboardButtonPageDown;
		case VK_END:
			return KeyCode::KeyboardButtonEnd;
		case VK_HOME:
			return KeyCode::KeyboardButtonHome;
		case VK_LEFT:
			return KeyCode::KeyboardButtonLeft;
		case VK_UP:
			return KeyCode::KeyboardButtonUp;
		case VK_RIGHT:
			return KeyCode::KeyboardButtonRight;
		case VK_DOWN:
			return KeyCode::KeyboardButtonDown;
		case VK_PRINT:
			return KeyCode::KeyboardButtonPrintScreen;
		case VK_SNAPSHOT:
			return KeyCode::KeyboardButtonPrintScreen;
		case VK_INSERT:
			return KeyCode::KeyboardButtonInsert;
		case VK_DELETE:
			return KeyCode::KeyboardButtonDelete;
		case VK_NUMPAD0:
		case '0':
			return KeyCode::KeyboardButtonAlpha0;
		case VK_NUMPAD1:
		case '1':
			return KeyCode::KeyboardButtonAlpha1;
		case VK_NUMPAD2:
		case '2':
			return KeyCode::KeyboardButtonAlpha2;
		case VK_NUMPAD3:
		case '3':
			return KeyCode::KeyboardButtonAlpha3;
		case VK_NUMPAD4:
		case '4':
			return KeyCode::KeyboardButtonAlpha4;
		case VK_NUMPAD5:
		case '5':
			return KeyCode::KeyboardButtonAlpha5;
		case VK_NUMPAD6:
		case '6':
			return KeyCode::KeyboardButtonAlpha6;
		case VK_NUMPAD7:
		case '7':
			return KeyCode::KeyboardButtonAlpha7;
		case VK_NUMPAD8:
		case '8':
			return KeyCode::KeyboardButtonAlpha8;
		case VK_NUMPAD9:
		case '9':
			return KeyCode::KeyboardButtonAlpha9;
		case VK_MULTIPLY:
			return KeyCode::KeyboardButtonMultiply;
		case VK_ADD:
			return KeyCode::KeyboardButtonAdd;
		case VK_SUBTRACT:
			return KeyCode::KeyboardButtonSubtract;
		case VK_DECIMAL:
			return KeyCode::KeyboardButtonDecimal;
		case VK_DIVIDE:
			return KeyCode::KeyboardButtonDivide;
		case VK_F1:
			return KeyCode::KeyboardButtonF1;
		case VK_F2:
			return KeyCode::KeyboardButtonF2;
		case VK_F3:
			return KeyCode::KeyboardButtonF3;
		case VK_F4:
			return KeyCode::KeyboardButtonF4;
		case VK_F5:
			return KeyCode::KeyboardButtonF5;
		case VK_F6:
			return KeyCode::KeyboardButtonF6;
		case VK_F7:
			return KeyCode::KeyboardButtonF7;
		case VK_F8:
			return KeyCode::KeyboardButtonF8;
		case VK_F9:
			return KeyCode::KeyboardButtonF9;
		case VK_F10:
			return KeyCode::KeyboardButtonF10;
		case VK_F11:
			return KeyCode::KeyboardButtonF11;
		case VK_F12:
			return KeyCode::KeyboardButtonF12;
		default:
			return (KeyCode)keyCode;
		}
	}
}
#endif

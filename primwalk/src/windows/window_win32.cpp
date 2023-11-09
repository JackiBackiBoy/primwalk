// primwalk
#include "../window.hpp"
#include "resource.hpp"
#include "../common/input/keycode.hpp"
#include "../common/input/rawInput.hpp"
#include "../common/ui/GUI.hpp"

// std
#include <iostream>
#include <cassert>
#include <stdexcept>

// windows
#include <windowsx.h>

namespace pw {
	WindowWin32::WindowWin32(const std::string& name, int width, int height)
		: WindowBase(name, width, height) {
		init();
	}

	int WindowWin32::init() {
		m_Instance = GetModuleHandle(0);
		m_Icon = LoadIcon(m_Instance, MAKEINTRESOURCE(IDI_APP_ICON));
		m_IconSmall = LoadIcon(m_Instance, MAKEINTRESOURCE(IDI_APP_ICON));

		// Convert string name to wide string
		std::wstring wName = stringToWideString(m_Name);

		// 1. Setup window class attributes
		WNDCLASSEX wcex{};
		wcex.cbSize = sizeof(wcex);							// Window class size in bytes
		wcex.style = CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW | CS_DBLCLKS;  // Window class styles
		wcex.lpszClassName = wName.c_str();					// Window class name
		wcex.hbrBackground = NULL;							// Window background brush color
		wcex.hCursor = LoadIcon(NULL, IDC_ARROW);			// Window cursor
		wcex.lpfnWndProc = WindowProc;						// Window procedure (message handler)
		wcex.hInstance = m_Instance;						// Window application instance
		wcex.hIcon = m_Icon;								// Window application icon
		wcex.hIconSm = m_IconSmall;							// Window application small icon

		// 2. Register window and ensure registration success
		if (!RegisterClassEx(&wcex)) {
			throw std::runtime_error("WIN32 ERROR: Failed to register window class!");
		}

		// 3. Setup window initialization attributes and create window
		m_Handle = CreateWindowEx(
			WS_EX_COMPOSITED | WS_EX_APPWINDOW,                    // Window extended styles
			wName.c_str(),        // Window class name
			wName.c_str(),        // Window title
			WS_OVERLAPPEDWINDOW,  // Window style
			CW_USEDEFAULT,        // Window X position
			CW_USEDEFAULT,        // Window Y position
			m_Width,              // Window width
			m_Height,             // Window height
			NULL,                 // Window parent
			NULL,                 // Window menu
			m_Instance,           // Window instance
			this                  // Additional data (Hack: this allows for early access to the window pointer)
		);

		// Validate window
		if (!m_Handle) { return 0; }

		pw::input::initialize();

		return 1;
	}

	UIEvent WindowWin32::createMouseEvent(unsigned int message, uint64_t wParam, int64_t lParam) {
		UIEvent event = UIEvent(UIEventType::MouseMove);
		MouseEventData& mouse = event.getMouseData();

		// Acquire mouse position
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

		if (message == WM_MOUSEWHEEL || message == WM_MOUSEHWHEEL) {
			ScreenToClient(m_Handle, &pt);
		}

		mouse.position = { pt.x, pt.y };

		// Acquire mouse button states
		mouse.downButtons.leftButton = (GET_KEYSTATE_WPARAM(wParam) & MK_LBUTTON) > 0;
		mouse.downButtons.middleButton = (GET_KEYSTATE_WPARAM(wParam) & MK_MBUTTON) > 0;
		mouse.downButtons.rightButton = (GET_KEYSTATE_WPARAM(wParam) & MK_RBUTTON) > 0;

		if (message == WM_MOUSEWHEEL) {
			mouse.wheelDelta.y = GET_WHEEL_DELTA_WPARAM(wParam) * 10.0f / WHEEL_DELTA;
		}
		else if (message == WM_MOUSEHWHEEL) {
			mouse.wheelDelta.x = GET_WHEEL_DELTA_WPARAM(wParam) * 10.0f / WHEEL_DELTA;
		}

		// Check which mouse buttons caused the mouse event (if any)
		switch (message) {
		case WM_LBUTTONUP:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
			mouse.causeButtons.leftButton = true;
			break;
		case WM_RBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
			mouse.causeButtons.rightButton = true;
			break;
		case WM_MBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONDBLCLK:
			mouse.causeButtons.middleButton = true;
			break;
		case WM_MOUSEMOVE:
			if (m_MouseButtonEvent.getType() == UIEventType::MouseDown) {
				mouse.causeButtons = m_MouseButtonEvent.getMouseData().causeButtons;
			}
			break;
		}

		bool buttonPressed = mouse.downButtons.leftButton || mouse.downButtons.middleButton || mouse.downButtons.rightButton;

		// Set event types accordingly
		switch (message) {
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		case WM_XBUTTONUP:
			{
				event.setType(UIEventType::MouseUp);
				event.getMouseData().clickCount = 0;

				if (!buttonPressed) {
					ReleaseCapture();
				}
			}
			break;
		case WM_LBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_XBUTTONDBLCLK:
			{
				event.setType(UIEventType::MouseDown);
				event.getMouseData().clickCount = 2;
			}
			break;
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_XBUTTONDOWN:
			{
				event.setType(UIEventType::MouseDown);
				event.getMouseData().clickCount = 1;
				SetCapture(m_Handle);
			}
			break;
		case WM_MOUSEMOVE:
			{
				// Hack: The Windows OS sends a WM_MOUSEMOVE message together with mouse down
				// flag when the window becomes the foreground window. Thus, we must also
				// check whether the mouse down event is the first since regaining the
				// foreground window focus. When handling WM_SETFOCUS we set the
				// m_EnteringWindow flag to 'true' which indicates this.
				// If that is the case, then we will not interpret that as a mouse-drag event
				// and then reset m_EnteringWindow to 'false'.
				event.setType((buttonPressed && !m_EnteringWindow) ? UIEventType::MouseDrag : UIEventType::MouseMove);

				if (m_EnteringWindow) {
					m_EnteringWindow = false;
				}
			}
			break;
		case WM_MOUSEWHEEL:
		case WM_MOUSEHWHEEL:
			{
				event.setType(UIEventType::MouseWheel);
			}
			break;
		}

		if (!m_TrackingMouseLeave && message != WM_MOUSELEAVE && message != WM_NCMOUSELEAVE && message != WM_NCMOUSEMOVE) {
			TRACKMOUSEEVENT tme{};
			tme.cbSize = sizeof(tme);
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = m_Handle;
			tme.dwHoverTime = HOVER_DEFAULT;

			TrackMouseEvent(&tme);
			m_TrackingMouseLeave = true;
		}

		// Store last occurrence of button press/release
		if (event.getType() == UIEventType::MouseDown ||
			event.getType() == UIEventType::MouseUp ||
			event.getType() == UIEventType::MouseExitWindow) {
			m_MouseButtonEvent = event;
		}

		return event;
	}

	void WindowWin32::processEvent(const UIEvent& event) {
		bool wasHandled = pw::gui::processEvent(event);
	}

	bool WindowWin32::isCursorInTitleBar(int x, int y) const {
		return x < m_Width - 90 && y < 29;
	}

	bool WindowWin32::isCursorOnBorder(int x, int y) const {
		RECT windowRect;
		GetWindowRect(m_Handle, &windowRect);

		return (x <= windowRect.right && x > windowRect.right - 8) ||
			(x >= windowRect.left && x < windowRect.left + 8) ||
			(y >= windowRect.top && y < windowRect.top + 8) ||
			(y <= windowRect.bottom && y > windowRect.bottom + 8);
	}

	void WindowWin32::pollEvents() {
		MSG msg = {};

		while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				// NOTE: While GLFW does not itself post WM_QUIT, other processes
				//       may post it to this one, for example Task Manager
				// HACK: Treat WM_QUIT as a close on all windows
				break;
			}
			else {
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
		}
	}

	LRESULT CALLBACK WindowWin32::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		WindowWin32* window = reinterpret_cast<WindowWin32*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		LRESULT result = 0;
		bool wasHandled = false;

		switch (message) {
		case WM_NCCREATE:
			{
				LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
				SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
			}
			break;
		case WM_NCHITTEST:
		{
			if (window->m_Fullscreen) {
				return HTCLIENT;
			}

			result = DefWindowProc(hWnd, WM_NCHITTEST, wParam, lParam);
			wasHandled = true;

			MouseCursor cursor = MouseCursor::None;

			if (result == HTCLIENT) {
				cursor = MouseCursor::Default;
				POINT mousePos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
				ScreenToClient(hWnd, &mousePos);
				auto target = pw::gui::hitTest({ mousePos.x, mousePos.y });

				if (target != nullptr) {
					cursor = target->getCursor();
				}
			}

			window->setCursor(cursor);
		}
		break;
		case WM_ERASEBKGND:
			return 1;
		case WM_PAINT:
			return 0;
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_XBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		case WM_XBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_XBUTTONDBLCLK:
		case WM_MOUSEWHEEL:
		case WM_MOUSEHWHEEL:
		case WM_MOUSEMOVE:
		case WM_MOUSELEAVE:
			{
				window->processEvent(window->createMouseEvent(message, wParam, lParam));
			}
			break;
		case WM_KEYDOWN:
			{
				UIEvent event(UIEventType::KeyboardDown);
				auto& data = event.getKeyboardData();
				data.pressedKey = pw::input::toKeyCode(wParam);

				// Keyboard modifiers
				static_assert(std::is_signed_v<decltype(GetAsyncKeyState(VK_SHIFT))>);
				auto r = KeyModifier::None;

				if (GetAsyncKeyState(VK_SHIFT) < 0) {
				r |= KeyModifier::Shift;
				}
				if (GetAsyncKeyState(VK_CONTROL) < 0) {
				r |= KeyModifier::Control;
				}
				if (GetAsyncKeyState(VK_MENU) < 0) {
				r |= KeyModifier::Alt;
				}
				if (GetAsyncKeyState(VK_LWIN) < 0 || GetAsyncKeyState(VK_RWIN) < 0) {
				r |= KeyModifier::Super;
				}

				data.modifier = r;

				window->processEvent(event);
			}
			break;
		case WM_CHAR:
			{
				if (wParam > 31) { // Only pass codepoints that are valid characters
					UIEvent event(UIEventType::KeyboardChar);
					event.getCharData().codePoint = static_cast<uint32_t>(wParam);
					window->processEvent(event);
				}
			}
			break;
		case WM_INPUT:
			{
				// Raw input handling
				pw::input::rawinput::parseMessage((void*)lParam);
			}
			break;
		case WM_KILLFOCUS:
		{
			window->processEvent({ UIEventType::FocusLost });
		}
		break;
		case WM_SETCURSOR:
		{
			if (LOWORD(lParam) == HTCLIENT) {
				result = 1;
				wasHandled = true;
			}
		}
		break;
		case WM_SIZE:
			{
				// This message is sent after WM_SIZING, which indicates that
				// the resizing operation is complete, this means that we can
				// safely render a frame until WM_SIZING is sent again.
				// Calculate new window dimensions if resized
				window->m_Width = LOWORD(lParam);
				window->m_Height = HIWORD(lParam);
				window->m_ResizeCallback(window->m_Width, window->m_Height);
			}
			break;
		case WM_DESTROY:
			{
				window->m_CloseFlag.store(true);
				PostQuitMessage(0);

				wasHandled = true;
				result = 1;
			}
			break;
		}

		if (!wasHandled) {
			result = DefWindowProc(hWnd, message, wParam, lParam);
		}

		return result;
	}

	void WindowWin32::setCursor(MouseCursor cursor) {
		m_Cursor = cursor;

		if (cursor == MouseCursor::None) {
			return;
		}

		static auto idcAppStarting = LoadCursor(nullptr, IDC_APPSTARTING);
		static auto idcArrow = LoadCursor(nullptr, IDC_ARROW);
		static auto idcHand = LoadCursor(nullptr, IDC_HAND);
		static auto idcIBeam = LoadCursor(nullptr, IDC_IBEAM);
		static auto idcNo = LoadCursor(nullptr, IDC_NO);

		auto idc = idcNo;
		switch (cursor) {
		case MouseCursor::None:
			idc = idcAppStarting;
			break;
		case MouseCursor::Default:
			idc = idcArrow;
			break;
		case MouseCursor::Hand:
			idc = idcHand;
			break;
		case MouseCursor::IBeam:
			idc = idcIBeam;
			break;
		}

		SetCursor(idc);
	}

	void WindowWin32::close() {
		m_CloseFlag.store(true);
		PostMessage(m_Handle, WM_CLOSE, 0, 0);
	}

	bool WindowWin32::shouldClose() {
		return m_CloseFlag.load(std::memory_order_relaxed);
	}

}

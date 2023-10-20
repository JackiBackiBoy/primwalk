// primwalk
#include "primwalk/window.hpp"
#include "primwalk/windows/resource.hpp"
#include "primwalk/input/keycode.hpp"
#include "primwalk/input/rawInput.hpp"
#include "primwalk/ui/GUI.hpp"

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

  int WindowWin32::run() {
    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    // Convert string name to wide string
    UnregisterClass(stringToWideString(m_Name).c_str(), m_Instance);
    return (int)msg.wParam;
  }

  int WindowWin32::init() {
    m_Instance = GetModuleHandle(0);
    m_Icon = LoadIcon(m_Instance, MAKEINTRESOURCE(IDI_APP_ICON));
    m_IconSmall = LoadIcon(m_Instance, MAKEINTRESOURCE(IDI_APP_ICON));

    // Convert string name to wide string
    std::wstring wName = stringToWideString(m_Name);

    // 1. Setup window class attributes
    WNDCLASSEX wcex{};
    wcex.cbSize        = sizeof(wcex);              // WNDCLASSEX size in bytes
    wcex.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;                // Window class styles
    wcex.lpszClassName = wName.c_str();             // Window class name
    wcex.hbrBackground = NULL;                      // Window background brush color
    wcex.hCursor       = LoadIcon(NULL, IDC_ARROW); // Window cursor
    wcex.lpfnWndProc   = WindowProc;                // Window procedure (message handler)
    wcex.hInstance     = m_Instance;                // Window application instance
    wcex.hIcon         = m_Icon;                    // Window application icon
    wcex.hIconSm       = m_IconSmall;               // Window application small icon

    // 2. Register window and ensure registration success
    if (!RegisterClassEx(&wcex)) {
      throw std::runtime_error("WIN32 ERROR: Failed to register window class!");
    }

    // 3. Setup window initialization attributes and create window
    m_Handle = CreateWindowEx(
      0,                    // Window extended styles
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

    return 1;
  }

  UIEvent WindowWin32::createMouseEvent(unsigned int message, uint64_t wParam, int64_t lParam)
  {
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
    case WM_MOUSELEAVE:
    case WM_NCMOUSELEAVE:
      {
        event.setType(UIEventType::MouseExitWindow);
        m_TrackingMouseLeave = false;
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

  void WindowWin32::processEvent(const UIEvent& event)
  {
    bool wasHandled = pw::gui::processEvent(event);

    if (!wasHandled) {
      switch (event.getType()) {
      case UIEventType::MouseDown:
      {
        auto& mouse = event.getMouseData();
        if (isCursorInTitleBar(mouse.position.x, mouse.position.y)) {
          if (mouse.causeButtons.leftButton && !m_Fullscreen) {
            if (mouse.clickCount == 1) {
              ReleaseCapture();
              SendMessage(m_Handle, WM_SYSCOMMAND, 0xf012, 0); // Hack: Use of undocumented flag "SC_DRAGMOVE"
            }
            else if (mouse.clickCount == 2) { // caption bar double click)
              toggleMaximize();
            }
          }
        }
      }
      break;
      }
    }
  }

  bool WindowWin32::isCursorInTitleBar(int x, int y) const
  {
    return x < m_Width - 90 && y < 29;
  }

  bool WindowWin32::isCursorOnBorder(int x, int y) const
  {
    RECT windowRect;
    GetWindowRect(m_Handle, &windowRect);

    return (x <= windowRect.right && x > windowRect.right - 8) ||
           (x >= windowRect.left && x < windowRect.left + 8) ||
           (y >= windowRect.top && y < windowRect.top + 8) ||
           (y <= windowRect.bottom && y > windowRect.bottom + 8);
  }

  // Hit test the frame for resizing and moving.
  LRESULT WindowWin32::hitTest(HWND hWnd, WPARAM wParam, LPARAM lParam) const
  {
    POINT ptMouse = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    RECT rcWindow;
    GetWindowRect(hWnd, &rcWindow);

    uint8_t left = ptMouse.x < rcWindow.left + 8;
    uint8_t right = ptMouse.x > rcWindow.right - 8;
    uint8_t top = ptMouse.y < rcWindow.top + 8;
    uint8_t bottom = ptMouse.y > rcWindow.bottom - 8;
    uint8_t topLeft = left & top;
    uint8_t topRight = right & top;
    uint8_t bottomLeft = left & bottom;
    uint8_t bottomRight = right & bottom;

    uint8_t cornerBits = bottomRight << 3 | bottomLeft << 2 | topRight << 1 | topLeft;

    if (cornerBits) {
      return cornerBits + 3 * (4 - (cornerBits >> 3));
    }

    uint8_t sideBits = bottom << 3 | left << 2 | top << 1 | right;

    if (sideBits) {
      return sideBits + (10 - (0x04 & sideBits) - ((0x08 & sideBits) >> 3) * 3);
    }

    return HTCLIENT;
}

  LRESULT WindowWin32::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    WindowWin32* window = nullptr;
    window = reinterpret_cast<WindowWin32*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    LRESULT result = 0;
    bool wasHandled = false;

    switch (message) {
      case WM_NCCREATE:
        {
          // Set window pointer on create
          CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
          WindowWin32* window = reinterpret_cast<WindowWin32*>(pCreate->lpCreateParams);
          SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)window);

          RECT rcClient;
          GetWindowRect(hWnd, &rcClient);

          // Inform the application of the frame change
          SetWindowPos(hWnd,
            NULL,
            rcClient.left, rcClient.top,
            rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
            SWP_FRAMECHANGED);

          wasHandled = true;
          result = DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
	  case WM_ERASEBKGND:
	  {
		  result = 1;
		  wasHandled = true;
	  }
	  break;
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
      case WM_NCHITTEST:
        {
          if (window->m_Fullscreen) {
            return HTCLIENT;
          }

          result = window->hitTest(hWnd, wParam, lParam);
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
      case WM_NCCALCSIZE:
        {
          // Handling this message allows us to extend the client area of the window
          // into the title bar region. Which effectively makes the whole window
          // region accessible for drawing, including the title bar.

          if (window->isFullscreen()) {
            return 0;
          }

          UINT dpi = GetDpiForWindow(hWnd);
          int frameX = GetSystemMetricsForDpi(SM_CXFRAME, dpi);
          int frameY = GetSystemMetricsForDpi(SM_CYFRAME, dpi);
          int padding = GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);

          NCCALCSIZE_PARAMS* params = (NCCALCSIZE_PARAMS*)lParam;
          RECT* requestedClientRect = params->rgrc;

          bool isMaximized = false;
          WINDOWPLACEMENT placement = { 0 };
          placement.length = sizeof(WINDOWPLACEMENT);
          if (GetWindowPlacement(hWnd, &placement)) {
            isMaximized = placement.showCmd == SW_SHOWMAXIMIZED;
          }

          if (isMaximized) {
            int sizeFrameY = GetSystemMetricsForDpi(SM_CYSIZEFRAME, dpi);
            requestedClientRect->right -= frameY + padding;
            requestedClientRect->left += frameX + padding;
            requestedClientRect->top += sizeFrameY + padding;
            requestedClientRect->bottom -= frameY + padding;
          }
          else {
            // ------ Hack to remove the title bar (non-client) area ------
            // In order to hide the standard title bar we must change
            // the NCCALCSIZE_PARAMS, which dictates the title bar area.
            //
            // In Windows 10 we can set the top component to '0', which
            // in effect hides the default title bar.
            // However, for unknown reasons this does not work in
            // Windows 11, instead we are required to set the top
            // component to '1' in order to get the same effect.
            //
            // Thus, we must first check the Windows version before
            // altering the NCCALCSIZE_PARAMS structure.
            const int cxBorder = 1;
            const int cyBorder = 1;
            InflateRect((LPRECT)lParam, -cxBorder, -cyBorder);
          }

          result = 0;
          wasHandled = true;
        }
        break;
      case WM_KILLFOCUS:
        {
          window->processEvent({ UIEventType::FocusLost });
        }
        break;
      case WM_SETFOCUS:
        {
          if (window != nullptr) {
            window->m_EnteringWindow = true;
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
      case WM_SETCURSOR:
        {
          if (LOWORD(lParam) == HTCLIENT) {
            result = 1;
            wasHandled = true;
          }
        }
        break;
      case WM_SYSCOMMAND:
        {
          // Reset minimized flag upon window restore
          if (wParam == SC_RESTORE) {
            window->m_IsMinimized.store(false);
          }
        }
        break;
      case WM_CLOSE:
        {
          PostQuitMessage(0);
        }
        break;
      case WM_DESTROY:
        {
          window->m_IsMinimized.store(false);
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

  void WindowWin32::setCursor(MouseCursor cursor)
  {
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

  void WindowWin32::toggleFullscreen()
  {
    DWORD dwStyle = GetWindowLong(m_Handle, GWL_STYLE);

    // Enter fullscreen mode
    if (!m_Fullscreen.load()) {
      bool maximized = false;

      if (m_Maximized.load()) {
        maximized = true;
      }

      m_Maximized.store(false);
      m_Fullscreen.store(true);

      MONITORINFO mi = { sizeof(mi) };
      if (GetWindowPlacement(m_Handle, &g_wpPrev) &&
        GetMonitorInfo(MonitorFromWindow(m_Handle,
          MONITOR_DEFAULTTOPRIMARY), &mi)) {

        if (maximized) {
          //SetWindowLong(m_Handle, GWL_STYLE,
          //  dwStyle & ~WS_OVERLAPPEDWINDOW);
          SetWindowLongPtr(m_Handle, GWL_EXSTYLE, WS_EX_APPWINDOW);
          SetWindowLongPtr(m_Handle, GWL_STYLE, WS_OVERLAPPED | WS_VISIBLE);
        }

        SetWindowPos(m_Handle, HWND_TOP,
          mi.rcMonitor.left, mi.rcMonitor.top,
          mi.rcMonitor.right - mi.rcMonitor.left,
          mi.rcMonitor.bottom - mi.rcMonitor.top,
          SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
      }
    }
    // Exit fullscreen mode
    else {

      m_Fullscreen.store(false);
      SetWindowLongPtr(m_Handle, GWL_EXSTYLE, 0);
      SetWindowLongPtr(m_Handle, GWL_STYLE,
        dwStyle | WS_OVERLAPPEDWINDOW);
      SetWindowPlacement(m_Handle, &g_wpPrev);
      SetWindowPos(m_Handle, HWND_TOP, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
        SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
  }

  void WindowWin32::toggleMaximize()
  {
    if (!m_Fullscreen.load()) {
      m_Maximized.store(!m_Maximized.load());
      WINDOWPLACEMENT wp{};
      GetWindowPlacement(m_Handle, &wp);
      ShowWindow(m_Handle, wp.showCmd == SW_MAXIMIZE ? SW_RESTORE : SW_MAXIMIZE);

    }
  }

  void WindowWin32::close()
  {
    m_CloseFlag.store(true);
    PostMessage(m_Handle, WM_CLOSE, 0, 0);
  }

  bool WindowWin32::shouldClose()
  {
    return m_CloseFlag.load(std::memory_order_relaxed);
  }

}

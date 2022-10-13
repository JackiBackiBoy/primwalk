#include "fzui/core/window.hpp"
#include <cassert>
#include "fzui/resource.hpp"
#include "fzui/ui/win32ui.hpp"
#include "fzui/managers/fontManager.hpp"
#include <iostream>

namespace fz {
  Window::Window(const std::wstring& name, const int& width, const int& height) {
    assert(width > 0 && "ASSERTION FAILED: Width must be greater than 0");
    assert(height > 0 && "ASSERTION FAILED: Height must be greater than 0");

    m_Name = name;
    m_Width = width;
    m_Height = height;
  }

  Window::~Window() {}

  int Window::run(HINSTANCE hInstance) {
    init(hInstance);

    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    UnregisterClass(m_Name.c_str(), m_Instance);

    return (int)msg.wParam;
  }

  int Window::init(HINSTANCE hInstance) {
    m_Instance = hInstance;

    backgroundBrush = CreateSolidBrush(RGB(255, 255, 255));

    HICON icon = LoadIcon(m_Instance, MAKEINTRESOURCE(IDI_APP_ICON_SMALL));
    HICON iconSmall = LoadIcon(m_Instance, MAKEINTRESOURCE(IDI_APP_ICON_SMALL));
    HCURSOR cursor = LoadCursor(NULL, IDC_ARROW);
    // Setup window class attributes.
    WNDCLASSEX wcex;
    ZeroMemory(&wcex, sizeof(wcex));

    wcex.cbSize = sizeof(wcex); // WNDCLASSEX size in bytes
    wcex.style = CS_HREDRAW | CS_VREDRAW;   // Window class styles
    wcex.lpszClassName = m_Name.c_str(); // Window class name
    wcex.hbrBackground = backgroundBrush;  // Window background brush color.
    wcex.hCursor = cursor;    // Window cursor
    wcex.lpfnWndProc = WindowProc;    // Window procedure associated to this window class.
    wcex.hInstance = m_Instance; // The application instance.
    wcex.hIcon = icon;      // Application icon.
    wcex.hIconSm = iconSmall; // Application small icon.

    // Register window and ensure registration success.
    if (!RegisterClassEx(&wcex)) {
      return 0;
    }

    // Setup window initialization attributes.
    CREATESTRUCT cs;
    ZeroMemory(&cs, sizeof(cs));

    cs.x = CW_USEDEFAULT; // Window X position
    cs.y = CW_USEDEFAULT; // Window Y position
    cs.cx = 840;  // Window width
    cs.cy = 680;  // Window height
    cs.hInstance = m_Instance; // Window instance.
    cs.lpszClass = wcex.lpszClassName;    // Window class name
    cs.lpszName = m_Name.c_str();  // Window title
    cs.style = WS_OVERLAPPEDWINDOW;   // Window style
    cs.dwExStyle = 0;
    cs.lpCreateParams = this;

    // Create the window.
    m_Handle = CreateWindowEx(
      cs.dwExStyle,
      cs.lpszClass,
      cs.lpszName,
      cs.style,
      cs.x,
      cs.y,
      cs.cx,
      cs.cy,
      cs.hwndParent,
      cs.hMenu,
      cs.hInstance,
      this);

    // Validate window.
    if (!m_Handle) { return 0; }

    return 1;
  }

  void Window::onCreate(HWND hWnd) {}

  void Window::addUiElement(Win32UiElement* element) {
    // Create ui element from creation structure
    CREATESTRUCT cs = element->getCreateStruct();

    HWND elementHandle = CreateWindowEx(
      cs.dwExStyle,
      cs.lpszClass,
      cs.lpszName,  // Styles 
      cs.style,
      cs.x,
      cs.y,
      cs.cx,
      cs.cy,
      cs.hwndParent,
      (HMENU)(size_t)m_LastID,
      cs.hInstance, 
      cs.lpCreateParams);

    m_UiHandles.insert({ m_LastID, elementHandle });
    m_UiElements.insert({ m_LastID, element });
    m_LastID++;

    // Change display font for the UI element
    SendMessage(elementHandle, WM_SETFONT, (LPARAM)element->getFont(), TRUE);
  }

  LRESULT Window::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;
    Window* window = nullptr;
    window = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    if (message == WM_CREATE) {
      // Set window pointer on create
      CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
      Window* window = reinterpret_cast<Window*>(pCreate->lpCreateParams);
      SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window);
      window = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

      window->onCreate(hwnd);

      ShowWindow(hwnd, SW_SHOWDEFAULT);
      UpdateWindow(hwnd);
    }
    else {
      bool wasHandled = false;

      switch (message) {
        case WM_COMMAND:
        {
          switch (HIWORD(wParam)) {
            case BN_CLICKED:
            {
              int id = LOWORD(wParam);
              window->getUiElement(id)->onClick();

              break;
            }
          }

          break;
        }
        case WM_CTLCOLORDLG:
        {
          wasHandled = true;
          result = (INT_PTR)window->backgroundBrush;
          break;
        }
        case WM_CTLCOLORSTATIC:
        {
          wasHandled = true;
          result = 0;
          break;
        }
        case WM_CTLCOLORBTN:
        {
          HDC hdc = (HDC)wParam;
          POINT pt;
          HWND hwndCtl;

          SetBkMode(hdc,TRANSPARENT); // Ensure that "static" text doesn't use a solid fill

          pt.x = 0; pt.y = 0;
          MapWindowPoints(hwndCtl, hwnd, &pt, 1);
          SetBrushOrgEx(hdc, -pt.x, -pt.y, NULL);

          wasHandled = true;
          result = (INT_PTR)window->backgroundBrush;
          break;
        }
        case WM_DESTROY:
        {
          window->onDestroy();

          PostQuitMessage(0);

          wasHandled = true;
          result = 1;
          break;
        }
      }

      if (!wasHandled) {
        result = DefWindowProc(hwnd, message, wParam, lParam);
      }
    }

    return result;
  }
}

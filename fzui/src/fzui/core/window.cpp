#include "fzui/core/window.hpp"
#include <cassert>
#include "fzui/resource.hpp"
#include "fzui/managers/fontManager.hpp"
#include "fzui/ui/uiStyle.hpp"
#include "fzui/ui/win32/win32Utilities.hpp"
#include <iostream>
#include <uxtheme.h>
#include <olectl.h>

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

    Color backgroundColor = UiStyle::darkBackground;
    backgroundBrush = CreateSolidBrush(RGB(backgroundColor.r, backgroundColor.g, backgroundColor.b));

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

    SetWindowLongPtr(elementHandle, GWLP_USERDATA, (LONG_PTR)false); // hover
    SetWindowSubclass(elementHandle, (SUBCLASSPROC)TrackerProc, m_LastID - 1, (DWORD_PTR)m_Handle);
    SendMessage(elementHandle, WM_SETFONT, (LPARAM)element->getFont(), TRUE);
  }

  void Window::setDarkMode(const bool& flag) {
    m_DarkMode = flag;
  }

  LRESULT Window::TrackerProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubClass, DWORD_PTR dwRefData) {
    static bool isHovered = false;

    if (message == WM_MOUSEMOVE) { // Mouse is over the button
      if (!(bool)GetWindowLongPtr(hWnd, GWLP_USERDATA)) {
        TRACKMOUSEEVENT ev = {};
        ev.cbSize = sizeof(TRACKMOUSEEVENT);
        ev.dwFlags = TME_HOVER | TME_LEAVE;
        ev.hwndTrack = hWnd;
        ev.dwHoverTime = 1;
        TrackMouseEvent(&ev);

        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)true); // hover flag
      }
    }
    else if (message == WM_MOUSEHOVER) {
      RECT rc = getRelativeClientRect(hWnd, GetParent(hWnd));

      SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)true); // hover flag
      InvalidateRect(GetParent(hWnd), &rc, FALSE);
    }
    else if (message == WM_MOUSELEAVE) { // Mouse left the control area
      if ((bool)GetWindowLongPtr(hWnd, GWLP_USERDATA)) {
        RECT rc = getRelativeClientRect(hWnd, GetParent(hWnd));

        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)false); // hover flag
        InvalidateRect(GetParent(hWnd), &rc, FALSE);
      }
    }

    return DefSubclassProc(hWnd, message, wParam, lParam);
  }

  LRESULT Window::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    static HBRUSH buttonDefaultBrush = CreateSolidBrush(getColorRef(UiStyle::darkButtonDefaultColor));
    static HBRUSH buttonHoverBrush = CreateSolidBrush(getColorRef(UiStyle::darkButtonSelectColor));

    LRESULT result = 0;
    Window* window = nullptr;
    window = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    if (message == WM_CREATE) {
      // Set window pointer on create
      CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
      Window* window = reinterpret_cast<Window*>(pCreate->lpCreateParams);
      SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)window);
      window = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

      window->onCreate(hWnd);

      SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)window->backgroundBrush);

      ShowWindow(hWnd, SW_SHOWDEFAULT);
      UpdateWindow(hWnd);
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
        case WM_CTLCOLORBTN:
        {
          HDC hdcStatic = (HDC) wParam;
          SetTextColor(hdcStatic, RGB(255,255,255));
          SetBkColor(hdcStatic, RGB(0,0,0));
          return (INT_PTR)window->backgroundBrush;
          break;
        }
        case WM_DRAWITEM:
        {
          LPDRAWITEMSTRUCT pDIS = (LPDRAWITEMSTRUCT)lParam;
          HPEN borderPen = CreatePen(PS_INSIDEFRAME, 0, RGB(0, 0, 0));
          HGDIOBJ oldPen = SelectObject(pDIS->hDC, borderPen);
          HGDIOBJ oldBrush;

          bool hovering = (bool)GetWindowLongPtr(pDIS->hwndItem, GWLP_USERDATA);

          if (hovering) {
            std::cout << "Hovering" << std::endl;
          }

          if (hovering) {
            oldBrush = SelectObject(pDIS->hDC, buttonHoverBrush);
          }
          else {
            oldBrush = SelectObject(pDIS->hDC, buttonDefaultBrush);
          }


          // Rounded button
          RoundRect(pDIS->hDC, pDIS->rcItem.left, pDIS->rcItem.top, pDIS->rcItem.right, pDIS->rcItem.bottom, 5, 5);

          //Clean up
          SelectObject(pDIS->hDC, oldPen);
          SelectObject(pDIS->hDC, oldBrush);
          DeleteObject(borderPen);

          // Calculate button dimensions
          int buttonWidth = pDIS->rcItem.right - pDIS->rcItem.left;
          int buttonHeight = pDIS->rcItem.bottom - pDIS->rcItem.top;

          WCHAR staticText[128];
          int len = SendMessage(pDIS->hwndItem, WM_GETTEXT, ARRAYSIZE(staticText), (LPARAM)staticText);
          HFONT buttonFont = (HFONT)SendMessage(pDIS->hwndItem, WM_GETFONT, 0, 0);

          SIZE buttonDim;
          HFONT oldFont = (HFONT)SelectObject(pDIS->hDC, buttonFont);
          GetTextExtentPoint32(pDIS->hDC, staticText, len, &buttonDim);

          SetTextColor(pDIS->hDC, RGB(255, 255, 255));
          SetBkMode(pDIS->hDC, TRANSPARENT);
          TextOut(pDIS->hDC, buttonWidth / 2 - buttonDim.cx / 2, buttonHeight / 2 - buttonDim.cy / 2, staticText, len);

          wasHandled = true;
          result = TRUE;
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
        result = DefWindowProc(hWnd, message, wParam, lParam);
      }
    }

    return result;
  }
}

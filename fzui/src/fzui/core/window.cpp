// std
#include <cassert>
#include <iostream>

// FZUI
#include "fzui/core/window.hpp"
#include "fzui/resource.hpp"
#include "fzui/managers/fontManager.hpp"
#include "fzui/ui/uiStyle.hpp"
#include "fzui/ui/win32/win32Utilities.hpp"
#include "fzui/ui/win32/win32Button.hpp"
#include "fzui/ui/win32/win32HoverFlags.hpp"

// Windows
#include <uxtheme.h>
#include <olectl.h>
#include <dwmapi.h>
#include <windowsx.h>

namespace fz {
  Window::Window(const std::wstring& name, const int& width, const int& height, Window* parent) {
    assert(width > 0 && "ASSERTION FAILED: Width must be greater than 0");
    assert(height > 0 && "ASSERTION FAILED: Height must be greater than 0");

    m_Name = name;
    m_Width = width;
    m_Height = height;
    m_Parent = parent;
  }

  Window::~Window() {
    for (auto it = m_UiElements.begin(); it != m_UiElements.end(); it++) {
      delete it->second;
    }
  }

  int Window::run(HINSTANCE hInstance) {
    init(hInstance);

    // Message loop
    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    UnregisterClass(m_Name.c_str(), m_Instance);

    return (int)msg.wParam;
  }

  void Window::setWindowInfo(const WindowInfo& windowInfo) {
    m_WindowInfo = windowInfo;
  }

  void Window::setPosition(const int& x, const int& y) {
    m_PositionX = x;
    m_PositionY = y;
  }

  void Window::setBackground(const Color& color) {
    m_BackgroundColor = color;
  }

  int Window::getHeight() const {
    return m_Height;
  }

  WindowInfo Window::getWindowInfo() const {
    return m_WindowInfo;
  }

  HWND Window::getWindowHandle() const {
    return m_Handle;
  }

  HBRUSH Window::getBackgroundBrush() const {
    return m_BackgroundBrush;
  }

  int Window::init(HINSTANCE hInstance) {
    m_Instance = hInstance;

    m_BackgroundBrush = CreateSolidBrush(Win32Utilities::getColorRef(m_BackgroundColor));

    m_Icon = LoadIcon(m_Instance, MAKEINTRESOURCE(IDI_APP_ICON));
    m_IconSmall = LoadIcon(m_Instance, MAKEINTRESOURCE(IDI_APP_ICON));

    // Setup window class attributes.
    WNDCLASSEX wcex;
    ZeroMemory(&wcex, sizeof(wcex));

    wcex.cbSize = sizeof(wcex); // WNDCLASSEX size in bytes
    wcex.style = CS_HREDRAW | CS_VREDRAW;   // Window class styles
    wcex.lpszClassName = m_Name.c_str(); // Window class name
    wcex.hbrBackground = m_BackgroundBrush;  // Window background brush color.
    wcex.hCursor = LoadIcon(NULL, IDC_ARROW);    // Window cursor
    wcex.lpfnWndProc = WindowProc;    // Window procedure associated to this window class.
    wcex.hInstance = m_Instance; // The application instance.
    wcex.hIcon = m_Icon;      // Application icon.
    wcex.hIconSm = m_IconSmall; // Application small icon.

    // Register window and ensure registration success.
    if (!RegisterClassEx(&wcex)) {
      std::cout << "ERROR: Could not create main window!" << std::endl;
      return 0;
    }

    // Setup window initialization attributes.
    CREATESTRUCT cs;
    ZeroMemory(&cs, sizeof(cs));

    // Calculate required coordinate for centered window position
    if (m_PositionX == FZUI_DONTCARE && m_PositionY == FZUI_DONTCARE) {
      m_PositionX = GetSystemMetrics(SM_CXSCREEN) / 2 - m_Width / 2;
      m_PositionY = GetSystemMetrics(SM_CYSCREEN) / 2 - m_Height / 2;
    }

    cs.x = m_PositionX; // Window X position
    cs.y = m_PositionY; // Window Y position
    cs.cx = m_Width;  // Window width
    cs.cy = m_Height;  // Window height
    cs.hInstance = m_Instance; // Window instance.
    cs.lpszClass = wcex.lpszClassName;    // Window class name
    cs.lpszName = m_Name.c_str();  // Window title
    cs.style = m_Parent == nullptr ? WS_OVERLAPPEDWINDOW : WS_VISIBLE | WS_CHILDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_DISABLED; // Window style
    cs.dwExStyle = 0;
    cs.hMenu = NULL;
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
      m_Parent == nullptr ? NULL : m_Parent->getWindowHandle(),
      cs.hMenu,
      cs.hInstance,
      this);

    // Validate window.
    if (!m_Handle) { return 0; }

    // Initialize all sub-windows
    for (auto child : m_SubWindows) {
      child->init(m_Instance);
    }

    return 1;
  }

  void Window::onCreate(HWND hWnd) {
    m_Handle = hWnd;

    // Create all the UI elements
    for (auto& it : m_UiElements) {
      CREATESTRUCT cs = it.second->getCreateStruct();

      HWND elementHandle = CreateWindowEx(
        cs.dwExStyle,
        cs.lpszClass,
        cs.lpszName,  // Styles 
        cs.style,
        cs.x,
        cs.y,
        cs.cx,
        cs.cy,
        m_Handle,
        (HMENU)(size_t)it.first,
        m_Instance, 
        cs.lpCreateParams);

      std::cout << it.first << std::endl;
      m_UiHandles.insert({ it.first, elementHandle });

      // Button specific
      if (cs.style & BS_OWNERDRAW) {
        SetWindowLongPtr(elementHandle, GWLP_USERDATA, false); // hover
        SetWindowSubclass(elementHandle, (SUBCLASSPROC)TrackerProc, it.first, (DWORD_PTR)m_Handle);
      }

      // Set control font
      // TODO: Investigate usage of LPARAM as opposed to WPARAM
      SendMessage(elementHandle, WM_SETFONT, (LPARAM)it.second->getFont(), TRUE);
    }
  }

  void Window::addMenuItem(Win32MenuItem* menuItem) {
    SIZE buttonSize = Win32Utilities::calcReqButtonSize(menuItem->getText(), 8, 8, FontManager::getInstance().getFont("Segoe UI", 16, FW_NORMAL), m_MenuHandle);

    if (m_MenuHandle == NULL) {
      WNDCLASSEX wcex;
      ZeroMemory(&wcex, sizeof(wcex));

      wcex.cbSize = sizeof(wcex); // WNDCLASSEX size in bytes
      wcex.style = CS_HREDRAW | CS_VREDRAW;   // Window class styles
      wcex.lpszClassName = L"FZUI.MenuBar"; // Window class name
      wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);  // Window background brush color.
      wcex.hCursor = LoadCursor(NULL, IDC_ARROW);    // Window cursor
      wcex.lpfnWndProc = MenuProc;    // Window procedure associated to this window class.
      wcex.hInstance = m_Instance; // The application instance.
      wcex.hIcon = NULL;      // Application icon.
      wcex.hIconSm = NULL; // Application small icon.

      // Register window and ensure registration success.
      if (!RegisterClassEx(&wcex)) {
        std::cout << "ERROR: Could not create child window!" << std::endl;
      }

      // Setup window initialization attributes
      CREATESTRUCT cs;
      ZeroMemory(&cs, sizeof(cs));

      // Calculate required coordinate for centered window position
      cs.x = 35; // Window X position
      cs.y = m_WindowInfo.titleBarHeight / 2 - UiStyle::defaultMenuHeight / 2; // Window Y position
      cs.cx = buttonSize.cx;  // Window width
      cs.cy = UiStyle::defaultMenuHeight;  // Window height
      cs.hwndParent = m_Handle;
      cs.hInstance = m_Instance; // Window instance.
      cs.lpszClass = wcex.lpszClassName;    // Window class name
      cs.lpszName = L"";  // Window title
      cs.style = WS_VISIBLE | WS_CHILDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;   // Window style
      cs.dwExStyle = 0;
      cs.hMenu = NULL;
      cs.lpCreateParams = this;

      // Create the window
      m_MenuHandle = CreateWindowEx(
        cs.dwExStyle,
        cs.lpszClass,
        cs.lpszName,
        cs.style,
        cs.x,
        cs.y,
        0,
        cs.cy,
        cs.hwndParent,
        cs.hMenu,
        cs.hInstance,
        this);

      // Validate window.
      if (!m_MenuHandle) { std::cout << "ERROR: Could not validate child window!" << std::endl; }
    }

    // Resize menubar accordingly
    RECT rc;
    GetWindowRect(m_MenuHandle, &rc);
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;

    SetWindowPos(m_MenuHandle, 0, 0, 0, width + buttonSize.cx, height, SWP_NOMOVE | SWP_NOZORDER);

    // New menu item pointer
    //Win32Button itemButton = Win32Button(menuItem->getText().c_str(), width, 0, buttonSize.cx, UiStyle::defaultMenuHeight, m_MenuHandle);
    CREATESTRUCT cs = menuItem->getCreateStruct();

    HWND itemHandle = CreateWindowEx(
      cs.dwExStyle,
      cs.lpszClass,
      cs.lpszName,  // Styles 
      cs.style,
      width,
      0,
      buttonSize.cx,
      UiStyle::defaultMenuHeight,
      m_MenuHandle,
      (HMENU)(size_t)m_LastID,
      m_Instance, 
      cs.lpCreateParams);

    std::cout << m_LastID << std::endl;
    m_UiHandles.insert({ m_LastID, itemHandle });
    m_LastID++;

    // Button specific
    if (cs.style & BS_OWNERDRAW) {
      SetWindowLongPtr(itemHandle, GWLP_USERDATA, false); // hover
      SetWindowSubclass(itemHandle, (SUBCLASSPROC)TrackerProc, m_LastID - 1, (DWORD_PTR)m_Handle);
    }

    // Set control font
    // TODO: Investigate usage of LPARAM as opposed to WPARAM
    SendMessage(itemHandle, WM_SETFONT, (LPARAM)FontManager::getInstance().getFont("Segoe UI", 16, FW_NORMAL), TRUE);
  }

  void Window::addSubWindow(Window* subWindow) {
    subWindow->m_Parent = this;
    m_SubWindows.push_back(subWindow);
  }

  bool Window::hasParent() const {
    return m_Parent != nullptr;
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
      SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)true); // hover flag
      InvalidateRect(hWnd, NULL, false);
    }
    else if (message == WM_MOUSELEAVE) { // Mouse left the control area
      if ((bool)GetWindowLongPtr(hWnd, GWLP_USERDATA)) {
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)false); // hover flag
        InvalidateRect(hWnd, NULL, false);
      }
    }
    else if (message == WM_ERASEBKGND) {
      return 1;
    }

    return DefSubclassProc(hWnd, message, wParam, lParam);
  }

  // Hit test the frame for resizing and moving.
  LRESULT HitTestNCA(HWND hWnd, WPARAM wParam, LPARAM lParam)
  {
    // Get the point coordinates for the hit test.
    POINT ptMouse = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

    // Get the window rectangle.
    RECT rcWindow;
    GetWindowRect(hWnd, &rcWindow);

    // Determine if the hit test is for resizing. Default middle (1,1).
    USHORT uRow = 1;
    USHORT uCol = 1;
    bool fOnResizeBorder = false;

    // Determine if the point is at the top or bottom of the window.
    if (ptMouse.y >= rcWindow.top && ptMouse.y < rcWindow.top + 29)
    {
        fOnResizeBorder = (ptMouse.y <= rcWindow.top + 8);
        uRow = 0;
    }
    else if (ptMouse.y < rcWindow.bottom && ptMouse.y >= rcWindow.bottom)
    {
        uRow = 2;
    }

    // Determine if the point is at the left or right of the window.
    if (ptMouse.x >= rcWindow.left && ptMouse.x < rcWindow.left + 10)
    {
        uCol = 0; // left side
    }
    else if (ptMouse.x < rcWindow.right && ptMouse.x >= rcWindow.right)
    {
        uCol = 2; // right side
    }

    // Hit test (HTTOPLEFT, ... HTBOTTOMRIGHT)
    LRESULT hitTests[3][3] = 
    {
        { HTTOPLEFT,    fOnResizeBorder ? HTTOP : HTCAPTION,    HTTOPRIGHT },
        { HTLEFT,       HTNOWHERE,     HTRIGHT },
        { HTBOTTOMLEFT, HTBOTTOM, HTBOTTOMRIGHT },
    };

    return hitTests[uRow][uCol];
}

  LRESULT Window::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    static HBRUSH btnDefBrush = CreateSolidBrush(Win32Utilities::getColorRef(UiStyle::darkButtonDefaultColor));
    static HBRUSH btnHovBrush = CreateSolidBrush(Win32Utilities::getColorRef(UiStyle::darkButtonHoverColor));

    LRESULT result = 0;
    Window* window = nullptr;
    window = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    //WindowProperties windowProperties = window->getProperties();

    if (message == WM_CREATE) {
      // Set window pointer on create
      CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
      Window* window = reinterpret_cast<Window*>(pCreate->lpCreateParams);
      SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)window);
      window = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

      window->onCreate(hWnd);

      SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)window->getBackgroundBrush());

      if (!window->hasParent()) {
        RECT rcClient;
        GetWindowRect(hWnd, &rcClient);

        // Inform the application of the frame change
        SetWindowPos(hWnd, 
                     NULL, 
                     rcClient.left, rcClient.top,
                     rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
                     SWP_FRAMECHANGED);

        ShowWindow(hWnd, SW_SHOWDEFAULT);
        UpdateWindow(hWnd);
      }
    }
    else {
      bool wasHandled = false;

      switch (message) {
        case WM_NCHITTEST:
        {
          result = HitTestNCA(hWnd, wParam, lParam);
          wasHandled = true;
          break;
        }
        case WM_NCCALCSIZE:
        {
          if (wParam == TRUE) {
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

            Win32Utilities& instance = Win32Utilities::Instance();
            int top = instance.getWindowsVersionString() == "Windows 11" ? 1 : 0;

            LPNCCALCSIZE_PARAMS ncParams = (LPNCCALCSIZE_PARAMS) lParam;
              ncParams->rgrc[0].top += top;
              ncParams->rgrc[0].left += 0;
              ncParams->rgrc[0].bottom -= 0;
              ncParams->rgrc[0].right -= 0;
              return 0;

            wasHandled = true;
            result = 0;
          }

          break;
        }
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
          return (INT_PTR)window->getBackgroundBrush();
          break;
        }
        case WM_DRAWITEM:
        {
          LPDRAWITEMSTRUCT pDIS = (LPDRAWITEMSTRUCT)lParam;

          // Button Owner Drawing
          if (pDIS->CtlType == ODT_BUTTON) {
            Win32Button* button = (Win32Button*)window->getUiElement(pDIS->CtlID);
            bool hovering = (bool)GetWindowLongPtr(pDIS->hwndItem, GWLP_USERDATA);
            bool hasBorder = button->getBorderThickness() > 0;

            // Border pen
            HPEN borderPen = NULL;
            HGDIOBJ oldPen = NULL;

            if (hasBorder) {
              borderPen = CreatePen(PS_INSIDEFRAME, 1, Win32Utilities::getColorRef(button->getBorderColor()));
            }
            else {
              borderPen = CreatePen(PS_INSIDEFRAME, 1, Win32Utilities::getColorRef(button->getDefaultColor()));
            }

            oldPen = SelectObject(pDIS->hDC, borderPen);

            HBRUSH buttonBrush;
            HGDIOBJ oldBrush;

            if (hovering) {
              buttonBrush = CreateSolidBrush(Win32Utilities::getColorRef(button->getHoverColor()));
            }
            else {
              buttonBrush = CreateSolidBrush(Win32Utilities::getColorRef(button->getDefaultColor()));
            }

            oldBrush = SelectObject(pDIS->hDC, buttonBrush);

            // Rounded button
            int borderDim = button->getBorderRadius() * 2; // width of ellipse
            RoundRect(pDIS->hDC, pDIS->rcItem.left, pDIS->rcItem.top,
                pDIS->rcItem.right, pDIS->rcItem.bottom, borderDim, borderDim);

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

            //Clean up
            SelectObject(pDIS->hDC, oldBrush);
            DeleteObject(buttonBrush);

            if (oldPen != NULL && borderPen != NULL) {
              SelectObject(pDIS->hDC, oldPen);
              DeleteObject(borderPen);
            }
          }
          else {
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
          }

          wasHandled = true;
          result = TRUE;
          break;
        }
        case WM_PAINT:
        {
          if (!window->hasParent()) { // check if window is top most
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);


            RECT rcClient;
            GetClientRect(hWnd, &rcClient);

            int width = rcClient.right - rcClient.left;
            int height = rcClient.bottom - rcClient.top;

            rcClient.bottom += window->getWindowInfo().titleBarHeight - height;

            // ------ Caption area ------
            HBRUSH captionBrush = CreateSolidBrush(Win32Utilities::getColorRef(UiStyle::darkCaptionColor));
            HGDIOBJ oldBrush = SelectObject(hdc, captionBrush);
            FillRect(hdc, &rcClient, captionBrush);

            // Application icon
            HICON appIcon = (HICON)LoadImage(NULL, L"assets/icons/fzcoach16x16.ico", IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
            DrawIconEx(hdc, 9, 6, appIcon, 16, 16, 0, NULL, DI_NORMAL);

            // Application title
            HFONT buttonFont = FontManager::getInstance().getFont("Segoe UI", 17, FW_NORMAL);

            HFONT oldFont = (HFONT)SelectObject(hdc, buttonFont);
            SIZE titleDim;
            GetTextExtentPoint32(hdc, L"Forza Coach", 11, &titleDim);
            int titleX = width / 2 - titleDim.cx / 2;

            SetTextColor(hdc, RGB(204, 204, 204));
            SetBkMode(hdc, TRANSPARENT);
            TextOut(hdc, titleX, 29 / 2 - titleDim.cy / 2, L"Forza Coach", 11);

            EndPaint(hWnd, &ps);

            // Cleanup
            SelectObject(hdc, oldBrush);
            DeleteObject(captionBrush);
            DestroyIcon(appIcon);
          }

          break;
        }
        case WM_SIZE:
          BufferedPaintStopAllAnimations(hWnd);
          break;
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

  LRESULT Window::MenuProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_DRAWITEM) {
      LPDRAWITEMSTRUCT pDIS = (LPDRAWITEMSTRUCT)lParam;

      // Button Owner Drawing
      if (pDIS->CtlType == ODT_BUTTON) {
        //Win32Button* button = (Win32Button*)window->getUiElement(pDIS->CtlID);
        bool hovering = (bool)GetWindowLongPtr(pDIS->hwndItem, GWLP_USERDATA);
        bool hasBorder = false;

        Color buttonColor = hovering ? UiStyle::darkButtonHoverColor : UiStyle::darkCaptionColor;

        // Border pen
        HPEN borderPen = NULL;
        HGDIOBJ oldPen = NULL;

        borderPen = CreatePen(PS_INSIDEFRAME, 1, Win32Utilities::getColorRef(buttonColor));
        oldPen = SelectObject(pDIS->hDC, borderPen);

        HBRUSH buttonBrush;
        HGDIOBJ oldBrush;

        buttonBrush = CreateSolidBrush(Win32Utilities::getColorRef(buttonColor));

        oldBrush = SelectObject(pDIS->hDC, buttonBrush);

        // Rounded button
        int borderDim = 0; // width of ellipse
        RoundRect(pDIS->hDC, pDIS->rcItem.left, pDIS->rcItem.top,
            pDIS->rcItem.right, pDIS->rcItem.bottom, borderDim, borderDim);

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

        //Clean up
        SelectObject(pDIS->hDC, oldBrush);
        DeleteObject(buttonBrush);

        if (oldPen != NULL && borderPen != NULL) {
          SelectObject(pDIS->hDC, oldPen);
          DeleteObject(borderPen);
        }
      }
      else {
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
      }

      return TRUE;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
  }
}

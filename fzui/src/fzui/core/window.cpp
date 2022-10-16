#include "fzui/core/window.hpp"
#include <cassert>
#include "fzui/resource.hpp"
#include "fzui/managers/fontManager.hpp"
#include "fzui/ui/uiStyle.hpp"
#include "fzui/ui/win32/win32Utilities.hpp"
#include <iostream>
#include <uxtheme.h>

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

    // Change display font for the UI element
    //SetWindowTheme(elementHandle, L"", L"");
    SendMessage(elementHandle, WM_SETFONT, (LPARAM)element->getFont(), TRUE);
  }

  void Window::setDarkMode(const bool& flag) {
    m_DarkMode = flag;
  }

  LRESULT Window::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    static HBRUSH defaultbrush = NULL;
    static HBRUSH hotbrush = NULL;
    static HBRUSH selectbrush = NULL;
    static HBRUSH buttonDefault = CreateSolidBrush(getColorRef(UiStyle::darkButtonDefaultColor));
    static HBRUSH buttonHoverBrush = CreateSolidBrush(getColorRef(UiStyle::darkButtonSelectColor));

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

      SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)window->backgroundBrush);

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
        case WM_NOTIFY:
        {
          LPNMHDR some_item = (LPNMHDR)lParam;

          std::cout << some_item->code << " : " << BCN_HOTITEMCHANGE << " : " << NM_CUSTOMDRAW << std::endl;

          if (some_item->code == BCN_HOTITEMCHANGE) { // mouse either entered or left button area
            NMBCHOTITEM* hotItem = reinterpret_cast<NMBCHOTITEM*>(lParam);
            HWND buttonHandle = some_item->hwndFrom;

            // Mouse entering
            if (hotItem->dwFlags & HICF_ENTERING) {
              NMHDR nmhdr{};
              nmhdr.hwndFrom = buttonHandle;
              nmhdr.idFrom = 0;
              nmhdr.code = NM_CUSTOMDRAW;
              //BeginBufferedAnimation(buttonHandle, 
            }
            // Mouse leaving
            else if (hotItem->dwFlags & HICF_LEAVING) {
            }
          }

          if (some_item->code == NM_CUSTOMDRAW)
          {
            LPNMCUSTOMDRAW item = (LPNMCUSTOMDRAW)some_item;

            if (item->uItemState & CDIS_SELECTED)
            {
              //Select our color when the button is selected
              if (selectbrush == NULL)
                  selectbrush = CreateSolidBrush(RGB(180, 0, 0));

              //Create pen for button border
              HPEN pen = CreatePen(PS_INSIDEFRAME, 0, RGB(0, 0, 0));

              //Select our brush into hDC
              HGDIOBJ old_pen = SelectObject(item->hdc, pen);
              HGDIOBJ old_brush = SelectObject(item->hdc, selectbrush);

              //If you want rounded button, then use this, otherwise use FillRect().
              RoundRect(item->hdc, item->rc.left, item->rc.top, item->rc.right, item->rc.bottom, 5, 5);

              //Clean up
              SelectObject(item->hdc, old_pen);
              SelectObject(item->hdc, old_brush);
              DeleteObject(pen);

              // Calculate button dimensions
              int buttonWidth = item->rc.right - item->rc.left;
              int buttonHeight = item->rc.bottom - item->rc.top;

              WCHAR staticText[128];
              int len = SendMessage(item->hdr.hwndFrom, WM_GETTEXT, ARRAYSIZE(staticText), (LPARAM)staticText);
              HFONT buttonFont = (HFONT)SendMessage(item->hdr.hwndFrom, WM_GETFONT, 0, 0);

              SIZE buttonDim;
              HFONT oldFont = (HFONT)SelectObject(item->hdc, buttonFont);
              GetTextExtentPoint32(item->hdc, staticText, len, &buttonDim);

              SetTextColor(item->hdc, RGB(255, 255, 255));
              TextOut(item->hdc, buttonWidth / 2 - buttonDim.cx / 2, buttonHeight / 2 - buttonDim.cy / 2, staticText, len);

              return CDRF_SKIPDEFAULT;
            }
            else {
              if (item->uItemState & CDIS_HOT) { //Our mouse is over the button
                //Select our color when the mouse hovers our button
                if (hotbrush == NULL)
                    hotbrush = CreateSolidBrush(RGB(255, 230, 0));

                HPEN pen = CreatePen(PS_INSIDEFRAME, 0, RGB(0, 0, 0));

                HGDIOBJ old_pen = SelectObject(item->hdc, pen);
                HGDIOBJ old_brush = SelectObject(item->hdc, hotbrush);

                RoundRect(item->hdc, item->rc.left, item->rc.top, item->rc.right, item->rc.bottom, 5, 5);

                SelectObject(item->hdc, old_pen);
                SelectObject(item->hdc, old_brush);
                DeleteObject(pen);

                // Calculate button dimensions
                int buttonWidth = item->rc.right - item->rc.left;
                int buttonHeight = item->rc.bottom - item->rc.top;

                WCHAR staticText[128];
                int len = SendMessage(item->hdr.hwndFrom, WM_GETTEXT, ARRAYSIZE(staticText), (LPARAM)staticText);
                HFONT buttonFont = (HFONT)SendMessage(item->hdr.hwndFrom, WM_GETFONT, 0, 0);

                SIZE buttonDim;
                HFONT oldFont = (HFONT)SelectObject(item->hdc, buttonFont);
                GetTextExtentPoint32(item->hdc, staticText, len, &buttonDim);

                SetTextColor(item->hdc, RGB(255, 255, 255));
                SetBkMode(item->hdc, TRANSPARENT);
                TextOut(item->hdc, buttonWidth / 2 - buttonDim.cx / 2, buttonHeight / 2 - buttonDim.cy / 2, staticText, len);

                return CDRF_SKIPDEFAULT;
              }

              //Select our color when our button is doing nothing
              if (defaultbrush == NULL) { defaultbrush = buttonDefault; }

              HPEN pen = CreatePen(PS_INSIDEFRAME, 0, RGB(0, 0, 0));

              HGDIOBJ old_pen = SelectObject(item->hdc, pen);
              HGDIOBJ old_brush = SelectObject(item->hdc, defaultbrush);

              RoundRect(item->hdc, item->rc.left, item->rc.top, item->rc.right, item->rc.bottom, 5, 5);

              SelectObject(item->hdc, old_pen);
              SelectObject(item->hdc, old_brush);
              DeleteObject(pen);

              // Calculate button dimensions
              int buttonWidth = item->rc.right - item->rc.left;
              int buttonHeight = item->rc.bottom - item->rc.top;

              WCHAR staticText[128];
              int len = SendMessage(item->hdr.hwndFrom, WM_GETTEXT, ARRAYSIZE(staticText), (LPARAM)staticText);
              HFONT buttonFont = (HFONT)SendMessage(item->hdr.hwndFrom, WM_GETFONT, 0, 0);

              SIZE buttonDim;
              HFONT oldFont = (HFONT)SelectObject(item->hdc, buttonFont);
              GetTextExtentPoint32(item->hdc, staticText, len, &buttonDim);

              SetTextColor(item->hdc, RGB(255, 255, 255));
              SetBkMode(item->hdc, TRANSPARENT);
              TextOut(item->hdc, buttonWidth / 2 - buttonDim.cx / 2, buttonHeight / 2 - buttonDim.cy / 2, staticText, len);

              return CDRF_SKIPDEFAULT;
            }
          }

          wasHandled = true;
          result = CDRF_SKIPDEFAULT;
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

          SetTextColor(pDIS->hDC, RGB(255, 255, 255));
          WCHAR staticText[99];
          int len = SendMessage(pDIS->hwndItem, WM_GETTEXT, ARRAYSIZE(staticText), (LPARAM)staticText);

          SetBkMode(pDIS->hDC, TRANSPARENT);
          TextOut(pDIS->hDC, pDIS->rcItem.left, pDIS->rcItem.top, staticText, len);

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
        result = DefWindowProc(hwnd, message, wParam, lParam);
      }
    }

    return result;
  }
}

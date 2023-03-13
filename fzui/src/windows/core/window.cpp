// std
#include <cassert>
#include <iostream>
#include <chrono>
#include <cmath>

// FZUI
#include "fzui/window.hpp"
#include "fzui/windows/resource.hpp"
#include "fzui/windows/win32/win32Utilities.hpp"
#include "fzui/windows/rendering/vertexArray.hpp"
#include "fzui/windows/rendering/vertexBuffer.hpp"
#include "fzui/windows/rendering/indexBuffer.hpp"
#include "fzui/windows/rendering/bufferLayout.hpp"
#include "fzui/windows/data/texture.hpp"
#include "fzui/windows/ui/uiButton.hpp"
#include "fzui/mouse.hpp"

// Windows
#include <sdkddkver.h>
#include <uxtheme.h>
#include <olectl.h>
#include <dwmapi.h>
#include <windowsx.h>

// Vendor
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0X2092
#define WGL_CONTEXT_FLAGS_ARB 0X2094
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
typedef HGLRC(WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC) (HDC hDC, HGLRC hShareContext, const int* attribList);
typedef const char* (WINAPI* PFNWGLGETEXTENSIONSSTRINGEXTPROC)(void);
typedef BOOL (WINAPI* PFNWGLSWAPINTERVALEXTPROC)(int);
typedef int (WINAPI* PFNWGLGETSWAPINTERVALEXTPROC) (void);

namespace fz {
  WindowWin32::WindowWin32(const std::wstring& name, const int& width, const int& height, WindowWin32* parent)
    : WindowBase() {
    assert(width > 0 && "ASSERTION FAILED: Width must be greater than 0");
    assert(height > 0 && "ASSERTION FAILED: Height must be greater than 0");

    m_Name = name;
    m_Width = width;
    m_Height = height;

    init();
  }

  WindowWin32::~WindowWin32() {
    delete m_Renderer2D;
  }

  int WindowWin32::run() {
    bool firstPaint = true;
    //HDC hdc = GetDC(m_Handle);

    // Application loop
    static float dt = 0.0f;
    auto currentTime = std::chrono::high_resolution_clock::now();

    

    // onCreate
    m_WindowIcon.loadFromFile("assets/textures/fzcoach16x16.png");
    m_MinmizeIcon.loadFromFile("assets/icons/minimize.png");
    m_MaximizeIcon.loadFromFile("assets/icons/maximize.png");
    m_CloseIcon.loadFromFile("assets/icons/close.png");

    MSG msg;
    while(true) {
      if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
          break;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }

      // TODO: Make frame timing consistent when resizing window
      auto newTime = std::chrono::high_resolution_clock::now();
      float dt = std::chrono::duration<float, std::chrono::seconds::period>(
          newTime - currentTime).count();
      currentTime = newTime;

      onUpdate(dt);
      onRender();

      if (firstPaint) {
        // Shows window
        ShowWindow(m_Handle, SW_SHOW);
        UpdateWindow(m_Handle);
        firstPaint = false;
      }
    }

    UnregisterClass(m_Name.c_str(), m_Instance);

    return (int)msg.wParam;
  }

  // UI
  void WindowWin32::addElement(UIElement* elem) {
    m_UIElements.push_back(elem);
  }

  int WindowWin32::init() {
    INITCOMMONCONTROLSEX icc{};

    // Initialise common controls.
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icc);

    m_Instance = GetModuleHandle(0);
    m_BackgroundBrush = CreateSolidBrush(Win32Utilities::getColorRef({ 30, 30, 30 }));
    m_Icon = LoadIcon(m_Instance, MAKEINTRESOURCE(IDI_APP_ICON));
    m_IconSmall = LoadIcon(m_Instance, MAKEINTRESOURCE(IDI_APP_ICON));

    // Setup window class attributes.
    WNDCLASSEX wcex;
    ZeroMemory(&wcex, sizeof(wcex));

    wcex.cbSize = sizeof(wcex); // WNDCLASSEX size in bytes
    wcex.style = CS_OWNDC;   // Window class styles
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

    cs.x = CW_USEDEFAULT; // Window X position
    cs.y = CW_USEDEFAULT; // Window Y position
    cs.cx = m_Width;  // Window width
    cs.cy = m_Height;  // Window height
    cs.hInstance = m_Instance; // Window instance.
    cs.lpszClass = wcex.lpszClassName; // Window class name
    cs.lpszName = m_Name.c_str(); // Window title
    cs.style = WS_OVERLAPPEDWINDOW; // Window style
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
      NULL,
      cs.hMenu,
      cs.hInstance,
      this);

    // Validate window.
    if (!m_Handle) { return 0; }

    createGraphicsContext();

    // Create the renderer once OpenGL is loaded
    m_Renderer2D = new Renderer2D();

    return 1;
  }

  void WindowWin32::createGraphicsContext() {
    m_HDC = GetDC(m_Handle);

    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 32;
    pfd.cStencilBits = 8;

    int pixelFormat = ChoosePixelFormat(m_HDC, &pfd);
    SetPixelFormat(m_HDC, pixelFormat, &pfd);

    // Set OpenGL rendering context
    HGLRC tempRC = wglCreateContext(m_HDC);
    wglMakeCurrent(m_HDC, tempRC);

    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

    // OpenGL version information
    const int attribList[] = {
      WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
      WGL_CONTEXT_MINOR_VERSION_ARB, 6,
      WGL_CONTEXT_FLAGS_ARB, 0,
      WGL_CONTEXT_PROFILE_MASK_ARB,
      WGL_CONTEXT_CORE_PROFILE_BIT_ARB, 0, 
    };

    HGLRC hglrc = wglCreateContextAttribsARB(m_HDC, 0, attribList);
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(tempRC);
    wglMakeCurrent(m_HDC, hglrc);

    if (!gladLoadGL())
    {
      printf("Could not initialize GLAD \n");
    }
    else {
      printf("OpenGL version: %i.%i\n", GLVersion.major, GLVersion.minor);
    }

    PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");
    bool swapControlSupported = strstr(_wglGetExtensionsStringEXT(), "WGL_EXT_swap_control") != 0;
    //https://www.khronos.org/opengl/wiki/Swap_Interval
    int vsync = 0;

    if (swapControlSupported) {
    PFNWGLSWAPINTERVALEXTPROC wglSwapInternalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
    PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");

    if (wglSwapInternalEXT(1)) {
      printf("VSync enabled \n");
    }
    else {
      printf("Could not enable VSync");
    }
    }
    else {
      printf("WGL_EXT_swap_control not supported \n");
    }
  }

  void WindowWin32::onCreate(HWND hWnd) {
    UIButton* button = new UIButton("Ok", { 50, 50 }, 100, 50);
    
    m_UIElements.push_back(button);
  }

  void WindowWin32::onUpdate(const float& dt) {

    // Get relative mouse position
    POINT p;
    GetCursorPos(&p);
    ScreenToClient(m_Handle, &p);

    Mouse& mouse = Mouse::Instance();
    mouse.m_RelativePos = { (float)p.x, (float)p.y };

    for (UIElement* element : m_UIElements) {
      element->update(dt); // TODO: Pass delta time instead
    }
  }

  void WindowWin32::onRender() {
    // Calculate new window dimensions if resized
    RECT clientRect;
    GetClientRect(m_Handle, &clientRect);
    m_Width = clientRect.right - clientRect.left;
    m_Height = clientRect.bottom - clientRect.top;
    float aspect = (float)m_Width / (float)m_Height;

    // TODO: Remove this call if possible
    m_Renderer2D->setViewport(m_Width, m_Height);

    m_Renderer2D->begin();

    // Render UI elements
    for (UIElement* element : m_UIElements) {
      element->draw(m_Renderer2D);
    }

    // ------ Caption bar area ------
    m_Renderer2D->drawRect(m_Width, 29, { 0.0f, 0.0f }, { 60, 60, 60 });
    //m_Renderer2D->drawRect(47, m_Height - 29, { 0.0f, 29.0f }, { 51, 51, 51 });
    m_Renderer2D->drawRect(16, 16, { 9.0f, 6.0f }, { 255, 255, 255 }, &m_WindowIcon);

    // Window minmize, maximize and close buttons
    m_Renderer2D->drawRect(30, 30, { m_Width - 90, 0.0f }, { 255, 255, 255 }, &m_MinmizeIcon);
    m_Renderer2D->drawRect(30, 30, { m_Width - 60, 0.0f }, { 255, 255, 255 }, &m_MaximizeIcon);
    m_Renderer2D->drawRect(30, 30, { m_Width - 30, 0.0f }, { 255, 255, 255 }, &m_CloseIcon);

    m_Renderer2D->drawText("Forza Coach (Beta)", { m_Width / 2 - 64, 29 / 2 - 15 / 2 }, 15, { 255, 255, 255 });

    glm::vec2 mousePos = Mouse::Instance().getRelativePos();
    std::string s = std::to_string((int)mousePos.x) + ", " + std::to_string((int)mousePos.y);

    static float time = 0.0f;
    time += 0.01f;
    m_Renderer2D->drawText(s, { 100, 100 }, ((sin(time) + 1.0f) / 2.0f + 1.0f) * 40, { 255, 255, 255 });

    m_Renderer2D->end();
    SwapBuffers(m_HDC);
  }

  // Hit test the frame for resizing and moving.
  LRESULT WindowWin32::HitTestNCA(HWND hWnd, WPARAM wParam, LPARAM lParam)
  {
    // Get the point coordinates for the hit test
    POINT ptMouse = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

    // Get the window rectangle.
    RECT rcWindow;
    GetWindowRect(hWnd, &rcWindow);

    // Determine if the hit test is for resizing. Default middle (1,1)
    USHORT uRow = 1;
    USHORT uCol = 1;
    bool fOnResizeBorder = false;

    // Determine if the point is at the top or bottom of the window
    if (ptMouse.y >= rcWindow.top && ptMouse.y < rcWindow.top + 29)
    {
      fOnResizeBorder = (ptMouse.y <= rcWindow.top + 8);
      uRow = 0;
    }
    else if (ptMouse.y < rcWindow.bottom && ptMouse.y >= rcWindow.bottom - 10)
    {
      uRow = 2;
    }

    // Determine if the point is on the left or right side of the window
    if (ptMouse.x >= rcWindow.left && ptMouse.x < rcWindow.left + 10)
    {
      uCol = 0; // left side
    }
    else if (ptMouse.x < rcWindow.right && ptMouse.x >= rcWindow.right - 10)
    {
      uCol = 2; // right side
    }

    // Maximize button
    if (uRow == 0 && ptMouse.x >= rcWindow.right - 60 && ptMouse.x < rcWindow.right - 30) {
      return HTMAXBUTTON;
    }

    if (ptMouse.x >= rcWindow.left && ptMouse.x < rcWindow.right &&
        ptMouse.y >= rcWindow.top && ptMouse.y < rcWindow.bottom &&
        uRow == 1 && uCol == 1) {
          return HTCLIENT;
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

  LRESULT WindowWin32::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;
    WindowWin32* window = nullptr;
    window = reinterpret_cast<WindowWin32*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    //WindowProperties windowProperties = window->getProperties();

    if (message == WM_CREATE) {
      // Set window pointer on create
      CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
      WindowWin32* window = reinterpret_cast<WindowWin32*>(pCreate->lpCreateParams);
      SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)window);
      window = reinterpret_cast<WindowWin32*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

      window->onCreate(hWnd);

      RECT rcClient;
      GetWindowRect(hWnd, &rcClient);

      // Inform the application of the frame change
      SetWindowPos(hWnd, 
                   NULL, 
                   rcClient.left, rcClient.top,
                   rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
                   SWP_FRAMECHANGED);

      //ShowWindow(hWnd, SW_SHOWDEFAULT);
      //UpdateWindow(hWnd);
    }
    else {
      bool wasHandled = false;

      switch (message) {
        case WM_ERASEBKGND:
        {
          result = 0;
          wasHandled = true;
          break;
        }
        case WM_NCHITTEST:
        {
          result = HitTestNCA(hWnd, wParam, lParam);
          wasHandled = true;
          break;
        }
        case WM_NCCALCSIZE:
        {
          const int cxBorder = 1;
          const int cyBorder = 1;
          InflateRect((LPRECT)lParam, -cxBorder, -cyBorder);

          return 0;
          break;
        }
        case WM_NCLBUTTONDOWN:
        {
          LRESULT result = 0;

          if (wParam == HTMAXBUTTON) {
            window->m_MaximizeButtonDown = true;
            return 0;
          }

          window->m_MaximizeButtonDown = false;

          break;
        }
        case WM_NCLBUTTONUP:
        {
          LRESULT result = 0;
          bool down = window->m_MaximizeButtonDown;
          

          if (wParam == HTMINBUTTON) {
            ShowWindow(hWnd, SW_MINIMIZE);
          }
          else if (wParam == HTMAXBUTTON && window->m_MaximizeButtonDown) {
            window->m_MaximizeButtonDown = false;
            WINDOWPLACEMENT wp{};
            GetWindowPlacement(hWnd, &wp);
            ShowWindow(hWnd, wp.showCmd == SW_MAXIMIZE ? SW_RESTORE : SW_MAXIMIZE);
          }
          else if (wParam == HTCLOSE) {
            SendMessage(hWnd, WM_DESTROY, 0, 0);
          }
          else {
            result = DefWindowProc(hWnd, message, wParam, lParam);
          }

          return result;

          break;
        }
        case WM_SIZING:
        {
          window->onRender();
          break;
        }
        case WM_SETCURSOR:
        {
          if (LOWORD(lParam) == HTCLIENT) {
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            return TRUE;
          }
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

// std
#include <cassert>
#include <iostream>
#include <cmath>
#include <thread>

// FZUI
#include "fzui/window.hpp"
#include "fzui/windows/resource.hpp"
#include "fzui/windows/win32/win32Utilities.hpp"
#include "fzui/rendering/vertexArray.hpp"
#include "fzui/rendering/vertexBuffer.hpp"
#include "fzui/rendering/indexBuffer.hpp"
#include "fzui/rendering/bufferLayout.hpp"
#include "fzui/data/texture.hpp"
#include "fzui/uiButton.hpp"
#include "fzui/mouse.hpp"
#include "fzui/data/fonts/fontManager.hpp"

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
  WindowWin32::WindowWin32(const std::string& name, const int& width, const int& height, WindowWin32* parent)
    : WindowBase() {
    assert(width > 0 && "ASSERTION FAILED: Width must be greater than 0");
    assert(height > 0 && "ASSERTION FAILED: Height must be greater than 0");

    m_Name = name;
    m_Width.store(width, std::memory_order_relaxed);
    m_Height.store(height, std::memory_order_relaxed);

    init();
  }

  WindowWin32::~WindowWin32() {
    delete m_Renderer2D;
  }

  int WindowWin32::run() {
    // Create separate rendering thread
    std::thread renderThread([this]() {
      renderingThread();
      });

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
      std::cout << msg.message << "\n";

      if (msg.message != 15) {
        
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }

    // Wait for the rendering thread to close
    renderThread.join();

    // Convert string name to wide string
    std::wstring wName(m_Name.size(), L' ');
    wName.resize(std::mbstowcs(&wName[0], m_Name.c_str(), m_Name.size()));
    UnregisterClass(wName.c_str(), m_Instance);

    return (int)msg.wParam;
  }

  // UI
  void WindowWin32::addElement(UIElement* elem) {
    m_UIElements.push_back(elem);
  }

  void WindowWin32::addContainer(UIContainer* container) {
    m_UIContainers.push_back(container);
  }

  int WindowWin32::init() {
    m_Instance = GetModuleHandle(0);
    m_Icon = LoadIcon(m_Instance, MAKEINTRESOURCE(IDI_APP_ICON));
    m_IconSmall = LoadIcon(m_Instance, MAKEINTRESOURCE(IDI_APP_ICON));

    // Convert string name to wide string
    std::wstring wName(m_Name.size(), L' ');
    wName.resize(std::mbstowcs(&wName[0], m_Name.c_str(), m_Name.size()));

    // Setup window class attributes.
    WNDCLASSEX wcex;
    ZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize = sizeof(wcex); // WNDCLASSEX size in bytes
    wcex.style = CS_OWNDC;   // Window class styles
    wcex.lpszClassName = wName.c_str(); // Window class name
    wcex.hbrBackground = NULL;  // Window background brush color.
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
    cs.cx = m_Width.load(std::memory_order_relaxed);  // Window width
    cs.cy = m_Height.load(std::memory_order_relaxed);  // Window height
    cs.hInstance = m_Instance; // Window instance.
    cs.lpszClass = wcex.lpszClassName; // Window class name
    cs.lpszName = wName.c_str(); // Window title
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
    pfd.cDepthBits = 24;
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

    hglrc = wglCreateContextAttribsARB(m_HDC, 0, attribList);
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

  void WindowWin32::renderingThread() {
    bool firstPaint = true;
    createGraphicsContext();
    m_Renderer2D = new Renderer2D();

    Window::onCreate();
    onCreate();

    auto lastTime = std::chrono::high_resolution_clock::now();

    // TODO: Make frame timing consistent when resizing window
    while (!m_ShouldClose.load(std::memory_order_relaxed)) {
      std::unique_lock<std::mutex> lock(mtx);
      cv.wait(lock, [&] { return !m_IsMinimized.load(std::memory_order_relaxed); });

      m_FrameDone.store(false, std::memory_order_relaxed);

      if (m_ShouldRender.load(std::memory_order_relaxed) &&
          !m_Resizing.load(std::memory_order_relaxed)) {
        wglMakeCurrent(m_HDC, hglrc);

        auto newTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::seconds::period>(
          newTime - lastTime).count();
        lastTime = newTime;

        // OnUpdate
        onUpdate(dt);
        Window::onUpdate(dt);

        // OnRender
        onRender(dt);
        Window::onRender(dt);

        // Extra resizing check to prevent in-loop buffer swapping
        if (!m_Resizing.load(std::memory_order_relaxed)) {
          SwapBuffers(m_HDC);
        }

        if (firstPaint) {
          // Shows window
          ShowWindow(m_Handle, SW_SHOW);
          UpdateWindow(m_Handle);
          firstPaint = false;
        }

        wglMakeCurrent(0, 0);
      }
      m_FrameDone.store(true, std::memory_order_relaxed);
    }
  }

  // ------ Event functions ------
  void WindowWin32::onCreate() {
    m_MinimizeIcon.loadFromFile("assets/icons/minimize.png");
    m_MaximizeIcon.loadFromFile("assets/icons/maximize.png");
    m_CloseIcon.loadFromFile("assets/icons/close.png");
  }

  void WindowWin32::onUpdate(float dt) {

    // Get relative mouse position
    POINT p;
    GetCursorPos(&p);
    ScreenToClient(m_Handle, &p);

    Mouse& mouse = Mouse::Instance();
    mouse.m_RelativePos = { (float)p.x, (float)p.y };

    for (UIContainer* container : m_UIContainers) {
      container->onUpdate(dt);
    }

    for (UIElement* element : m_UIElements) {
      element->update(dt);
    }
  }

  void WindowWin32::onRender(float dt) {
    // Calculate new window dimensions if resized
    RECT clientRect;
    GetClientRect(m_Handle, &clientRect);
    m_Width.store(clientRect.right - clientRect.left, std::memory_order_relaxed);
    m_Height.store(clientRect.bottom - clientRect.top, std::memory_order_relaxed);
    float aspect = (float)m_Width.load(std::memory_order_relaxed) / (float)m_Height.load(std::memory_order_relaxed);

    // TODO: Remove this call if possible
    glViewport(0, 0, m_Width.load(std::memory_order_relaxed), m_Height.load(std::memory_order_relaxed));
    m_Renderer2D->setViewport(m_Width.load(std::memory_order_relaxed), m_Height.load(std::memory_order_relaxed));

    m_Renderer2D->begin();

    // Render UI elements and containers
    for (UIContainer* container : m_UIContainers) {
      container->onRender(m_Renderer2D);
    }

    for (UIElement* element : m_UIElements) {
      element->draw(m_Renderer2D);
    }

    // ------ Caption bar area ------
    m_Renderer2D->drawRect(m_Width.load(std::memory_order_relaxed), 29, { 0.0f, 0.0f }, { 60, 60, 60, 200 });

    // Window minimize, maximize and close buttons
    m_Renderer2D->drawRect(30, 30, { m_Width - 90, 0.0f }, { 255, 255, 255 }, 0, &m_MinimizeIcon);
    m_Renderer2D->drawRect(30, 30, { m_Width - 60, 0.0f }, { 255, 255, 255 }, 0, &m_MaximizeIcon);
    m_Renderer2D->drawRect(30, 30, { m_Width - 30, 0.0f }, { 255, 255, 255 }, 0, &m_CloseIcon);

    m_Renderer2D->end();
  }

  int WindowWin32::getWidth() const {
    return m_Width.load(std::memory_order_relaxed);
  }

  int WindowWin32::getHeight() const {
    return m_Height.load(std::memory_order_relaxed);
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
    if (uRow == 0 && ptMouse.x >= rcWindow.right - 30 && ptMouse.x < rcWindow.right) {
      return HTCLOSE;
    }
    else if (uRow == 0 && ptMouse.x >= rcWindow.right - 60 && ptMouse.x < rcWindow.right - 30) {
      return HTMAXBUTTON;
    }
    else if (uRow == 0 && ptMouse.x >= rcWindow.right - 90 && ptMouse.x < rcWindow.right - 60) {
      return HTMINBUTTON;
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

    if (message == WM_NCCREATE) {
      // Set window pointer on create
      CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
      WindowWin32* window = reinterpret_cast<WindowWin32*>(pCreate->lpCreateParams);
      SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)window);
      window = reinterpret_cast<WindowWin32*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
      
      RECT rcClient;
      GetWindowRect(hWnd, &rcClient);

      // Inform the application of the frame change
      SetWindowPos(hWnd, 
                   NULL, 
                   rcClient.left, rcClient.top,
                   rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
                   SWP_FRAMECHANGED);

      result = DefWindowProc(hWnd, message, wParam, lParam);
    }
    else {
      bool wasHandled = false;

      switch (message) {
        case WM_ERASEBKGND:
        {
          return 1;
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
          
          if (wParam == HTMINBUTTON) {
            window->m_MinimizeButtonDown = true;
            return 0;
          }
          else if (wParam == HTMAXBUTTON) {
            window->m_MaximizeButtonDown = true;
            return 0;
          }
          if (wParam == HTCLOSE) {
            window->m_CloseButtonDown = true;
            return 0;
          }

          window->m_MaximizeButtonDown = false;
          //return 0;

          break;
        }
        case WM_NCLBUTTONUP:
        {
          LRESULT result = 0;
          bool down = window->m_MaximizeButtonDown;
          

          if (wParam == HTMINBUTTON) {
            window->m_IsMinimized.store(true, std::memory_order_relaxed);
            window->cv.notify_all();

            //window->m_IsMinimized.store(true, std::memory_order_relaxed);
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
        case WM_ENTERSIZEMOVE:
        {
          
          break;
        }
        case WM_EXITSIZEMOVE:
        {
          // In case WM_SIZE is not sent during fast resize
          window->m_Resizing.store(false, std::memory_order_relaxed);
          break;
        }
        case WM_SIZE:
        {
          // This message is sent after WM_SIZING, which indicates that
          // the resizing operation is complete, this means that we can
          // safely render a frame until WM_SIZING is sent again.
          window->m_Resizing.store(false, std::memory_order_relaxed);
          break;
        }
        case WM_SIZING:
        {
          // When WM_SIZING is sent the rendering thread should render
          // a frame as fast as possible and then lock rendering until
          // all related sizing operations have been completed, namely
          // until the WM_SIZE message is sent.
          while (!window->m_FrameDone.load(std::memory_order_relaxed)) {}; // wait for frame completion
          window->m_Resizing.store(true, std::memory_order_relaxed); // locks rendering
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
        case WM_SYSCOMMAND:
        {
          // Reset minimized flag upon window restore
          if (wParam == SC_RESTORE) {
            window->m_IsMinimized.store(false, std::memory_order_relaxed);
            window->cv.notify_all();
          }

          break;
        }
        case WM_DESTROY:
        {
          window->m_ShouldClose.store(true, std::memory_order_relaxed);
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

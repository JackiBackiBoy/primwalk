// std
#include <cassert>
#include <iostream>
#include <chrono>
#include <cmath>

// FZUI
#include "fzui/core/window.hpp"
#include "fzui/resource.hpp"
#include "fzui/win32/win32Utilities.hpp"
#include "fzui/rendering/vertexArray.hpp"
#include "fzui/rendering/vertexBuffer.hpp"
#include "fzui/rendering/indexBuffer.hpp"
#include "fzui/rendering/bufferLayout.hpp"
#include "fzui/data/texture.hpp"

// Windows
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
  Window::Window(const std::wstring& name, const int& width, const int& height, Window* parent) {
    assert(width > 0 && "ASSERTION FAILED: Width must be greater than 0");
    assert(height > 0 && "ASSERTION FAILED: Height must be greater than 0");

    m_Name = name;
    m_Width = width;
    m_Height = height;
  }

  Window::~Window() {
    delete m_Renderer2D;
  }

  int Window::run(HINSTANCE hInstance) {
    init(hInstance);

    bool firstPaint = true;
    //HDC hdc = GetDC(m_Handle);

    // Application loop
    static float dt = 0.0f;
    auto currentTime = std::chrono::high_resolution_clock::now();

    // Shaders
    shader.loadShader(ShaderType::Vertex, "assets/shaders/uiShader.vert");
    shader.loadShader(ShaderType::Fragment, "assets/shaders/uiShader.frag");
    shader.compileShaders();
    shader.bind();
    auto loc = glGetUniformLocation(shader.getID(), "u_Textures");
    int samplers[3] = { 0, 1, 2 };
    glUniform1iv(loc, 3, samplers);

    glm::mat4 projMat = glm::ortho(0.0f, (float)m_Width, (float)m_Height, 0.0f);
    shader.setUniformMat4("projMat", projMat);

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

  // Setters

  int Window::init(HINSTANCE hInstance) {
    m_Instance = hInstance;
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

  void Window::createGraphicsContext() {
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

  void Window::onCreate(HWND hWnd) {
  }

  void Window::onRender() {
    // Calculate new window dimensions if resized
    RECT clientRect;
    GetClientRect(m_Handle, &clientRect);
    m_Width = clientRect.right - clientRect.left;
    m_Height = clientRect.bottom - clientRect.top;
    float aspect = (float)m_Width / (float)m_Height;
    glm::mat4 projMat = glm::ortho(0.0f, (float)m_Width, (float)m_Height, 0.0f);

    glViewport(0, 0, m_Width, m_Height);

    shader.bind();
    shader.setUniformMat4("projMat", projMat);
    
    m_Renderer2D->begin();

    // Render UI elements
    for (auto& element : m_UIElements) {
      element.draw(m_Renderer2D);
    }

    // Caption bar area
    m_Renderer2D->drawRect(m_Width, 29, { 0.0f, 0.0f }, Color::normalize({ 60, 60, 60 }), 0);
    m_Renderer2D->drawRect(47, m_Height - 29, { 0.0f, 29.0f }, Color::normalize({ 51, 51, 51 }), 0);
    m_Renderer2D->drawRect(16, 16, { 9.0f, 6.0f }, Color::normalize({ 255, 255, 255 }), 1);

    m_Renderer2D->drawText("Forza Coach (Beta)", { m_Width / 2 - 64, 29 / 2 - 15 / 2 }, 15, Color::normalize({ 203, 203, 203 }));

    m_Renderer2D->end();
    SwapBuffers(m_HDC);
  }

  // Hit test the frame for resizing and moving.
  LRESULT HitTestNCA(HWND hWnd, WPARAM wParam, LPARAM lParam)
  {
    // Get the point coordinates for the hit test.
    POINT ptMouse = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

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
        case WM_SIZING:
        {
          window->onRender();
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

// std
#include <cassert>
#include <iostream>
#include <cmath>
#include <thread>

// FZUI
#include "fzui/window.hpp"
#include "fzui/windows/resource.hpp"
#include "fzui/windows/win32/win32Utilities.hpp"
#include "fzui/rendering/renderer.hpp"
#include "fzui/rendering/frameInfo.hpp"
#include "fzui/mouse.hpp"

// Windows
#include <sdkddkver.h>
#include <uxtheme.h>
#include <olectl.h>
#include <dwmapi.h>
#include <windowsx.h>

// vendor
#include <glm/gtc/matrix_transform.hpp>

namespace fz {
  WindowWin32::WindowWin32(const std::string& name, const int& width, const int& height, const GraphicsAPI& api, WindowWin32* parent)
    : WindowBase() {
    assert(width > 0 && "ASSERTION FAILED: Width must be greater than 0");
    assert(height > 0 && "ASSERTION FAILED: Height must be greater than 0");

    m_Name = name;
    m_Width.store(width, std::memory_order_relaxed);
    m_Height.store(height, std::memory_order_relaxed);
    m_API = api;

    init();
  }

  WindowWin32::~WindowWin32() {
  }

  int WindowWin32::run() {
    // Create separate rendering thread
    std::thread renderThread([this]() {
      renderingThread();
    });

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    // Wait for the rendering thread to close
    renderThread.join();

    // Convert string name to wide string
    UnregisterClass(Win32Utilities::stringToWideString(m_Name).c_str(), m_Instance);

    return (int)msg.wParam;
  }

  int WindowWin32::init() {
    m_Instance = GetModuleHandle(0);
    m_Icon = LoadIcon(m_Instance, MAKEINTRESOURCE(IDI_APP_ICON));
    m_IconSmall = LoadIcon(m_Instance, MAKEINTRESOURCE(IDI_APP_ICON));

    // Convert string name to wide string
    std::wstring wName = Win32Utilities::stringToWideString(m_Name);

    // 1. Setup window class attributes.
    WNDCLASSEX wcex;
    ZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize        = sizeof(wcex);              // WNDCLASSEX size in bytes
    wcex.style         = CS_OWNDC;                  // Window class styles
    wcex.lpszClassName = wName.c_str();             // Window class name
    wcex.hbrBackground = NULL;                      // Window background brush color
    wcex.hCursor       = LoadIcon(NULL, IDC_ARROW); // Window cursor
    wcex.lpfnWndProc   = WindowProc;                // Window procedure associated to this window class
    wcex.hInstance     = m_Instance;                // The application instance
    wcex.hIcon         = m_Icon;                    // Application icon
    wcex.hIconSm       = m_IconSmall;               // Application small icon

    // 2. Register window and ensure registration success.
    if (!RegisterClassEx(&wcex)) {
      std::cout << "ERROR: Could not create main window!" << std::endl;
      return 0;
    }

    // 3. Setup window initialization attributes
    POINT screenCenter = Win32Utilities::getScreenCenter();
    CREATESTRUCT cs;
    ZeroMemory(&cs, sizeof(cs));
    cs.x              = screenCenter.x - m_Width.load(std::memory_order_relaxed) / 2;                            // Window X position
    cs.y              = screenCenter.y - m_Height.load(std::memory_order_relaxed) / 2;                            // Window Y position
    cs.cx             = m_Width.load(std::memory_order_relaxed);  // Window width
    cs.cy             = m_Height.load(std::memory_order_relaxed); // Window height
    cs.hInstance      = m_Instance;                               // Window instance
    cs.lpszClass      = wcex.lpszClassName;                       // Window class name
    cs.lpszName       = wName.c_str();                            // Window title
    cs.style          = WS_OVERLAPPEDWINDOW;                      // Window style
    cs.dwExStyle      = 0;                                        // Window extended styles
    cs.hMenu          = NULL;                                     // Window menu
    cs.lpCreateParams = this;                                     // Window creation parameters

    // Create the window
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

    // Validate window
    if (!m_Handle) { return 0; }

    return 1;
  }

  void WindowWin32::createGraphicsContext(const GraphicsAPI& api) {
    m_HDC = GetDC(m_Handle);
    m_GraphicsDevice = std::make_shared<GraphicsDevice_Vulkan>(m_Handle);
    m_Renderer = std::make_unique<Renderer>(*this, *m_GraphicsDevice);

    if (api == GraphicsAPI::Vulkan) {
      
    }
  }

  void WindowWin32::renderingThread() {
    bool firstPaint = true;
    createGraphicsContext(m_API);
    //m_Renderer2D = new Renderer2D();

    Window::onCreate();
    onCreate();

    // TODO: Move to onCreate
    VkRenderPass renderPass = m_Renderer->getSwapChainRenderPass();
    std::vector<VkDescriptorSetLayout> setLayouts;
    m_UIRenderSystem = std::make_unique<UIRenderSystem>(*m_GraphicsDevice, renderPass, setLayouts);

    auto lastTime = std::chrono::high_resolution_clock::now();

    // TODO: Make frame timing consistent when resizing window
    while (!m_ShouldClose.load(std::memory_order_relaxed)) {

      m_FrameDone.store(false, std::memory_order_relaxed);

      if (m_ShouldRender.load(std::memory_order_relaxed)) {
        auto newTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::seconds::period>(
          newTime - lastTime).count();
        lastTime = newTime;

        if (m_ShouldRender.load(std::memory_order_relaxed)) {
        if (auto commandBuffer = m_Renderer->beginFrame()) {
          int frameIndex = m_Renderer->getFrameIndex();
          // Update
          FrameInfo frameInfo{};
          frameInfo.frameIndex = frameIndex;
          frameInfo.frameTime = dt;
          frameInfo.commandBuffer = commandBuffer;
          frameInfo.windowWidth = (float)m_Renderer->getSwapChainWidth();
          frameInfo.windowHeight = (float)m_Renderer->getSwapChainHeight();

          m_UIRenderSystem->onUpdate(frameInfo);

          // Render
          m_Renderer->beginSwapChainRenderPass(commandBuffer);
          m_UIRenderSystem->onRender(frameInfo);
          m_Renderer->endSwapChainRenderPass(commandBuffer);
          
          m_Renderer->endFrame();
        }

        m_FrameDone.store(true, std::memory_order_relaxed);

        if (firstPaint) {
          // Shows window
          ShowWindow(m_Handle, SW_SHOW);
          UpdateWindow(m_Handle);
          firstPaint = false;
        }
      }
      }
    }

    vkDeviceWaitIdle(m_GraphicsDevice->getDevice());
  }

  // ------ Event functions ------
  void WindowWin32::onCreate() {
    //m_MinimizeIcon.loadFromFile("assets/icons/minimize.png");
    //m_MaximizeIcon.loadFromFile("assets/icons/maximize.png");
    //m_CloseIcon.loadFromFile("assets/icons/close.png");
  }

  void WindowWin32::onUpdate(float dt) {
  }

  void WindowWin32::onRender(float dt) {
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
          return 0;
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

          break;
        }
        case WM_NCLBUTTONUP:
        {
          LRESULT result = 0;
          bool down = window->m_MaximizeButtonDown;
          

          if (wParam == HTMINBUTTON) {
            window->m_IsMinimized.store(true, std::memory_order_relaxed);
            window->m_RenderingCondition.notify_all();

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
        case WM_PAINT: {
          //if (window != nullptr && window->m_GraphicsDevice != nullptr && window->m_GraphicsPipeline != nullptr) {
            //window->m_GraphicsDevice->drawFrame(*window->m_GraphicsPipeline);
          //}
          return 0;
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
          // Calculate new window dimensions if resized
          window->m_Resizing.store(false, std::memory_order_relaxed);
          window->m_Width.store(LOWORD(lParam), std::memory_order_relaxed);
          window->m_Height.store(HIWORD(lParam), std::memory_order_relaxed);
          
          
          if (window->m_GraphicsDevice != nullptr) {
            //window->m_GraphicsDevice->framebufferResizeCallback();
            window->m_Renderer->m_FramebufferResized.load(std::memory_order_relaxed);
            //window->m_ShouldRender.store(true, std::memory_order_relaxed);
            while (!window->m_FrameDone.load(std::memory_order_relaxed)) {}; // wait for frame completion
          }

          break;
        }
        case WM_SIZING:
        {
          // When WM_SIZING is sent the rendering thread should render
          // a frame as fast as possible and then lock rendering until
          // all related sizing operations have been completed, namely
          // until the WM_SIZE message is sent.
          //while (!window->m_FrameDone.load(std::memory_order_relaxed)) {}; // wait for frame completion
          //window->m_Resizing.store(true, std::memory_order_relaxed); // locks rendering
          window->m_Resizing.store(true, std::memory_order_relaxed);
          //window->m_ShouldRender.store(false, std::memory_order_relaxed);
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
            window->m_RenderingCondition.notify_all();
          }

          break;
        }
        case WM_DESTROY:
        {
          window->m_IsMinimized.store(false, std::memory_order_relaxed);
          window->m_RenderingCondition.notify_all();
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

  HWND WindowWin32::getHandle() const {
    return m_Handle;
  }
}

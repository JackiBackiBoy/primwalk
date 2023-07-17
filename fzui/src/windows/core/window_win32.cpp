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
#include "fzui/rendering/graphicsDevice.hpp"
#include "fzui/rendering/graphicsPipeline.hpp"
#include "fzui/rendering/frameInfo.hpp"
#include "fzui/mouse.hpp"
#include "fzui/ui/uiIconButton.hpp"
#include "fzui/rendering/texture2D.hpp"

// vendor
#include <glm/gtc/matrix_transform.hpp>
#include <dwmapi.h>
#include <windowsx.h>

namespace fz {
  WindowWin32::WindowWin32(const std::string& name, int width, int height)
    : WindowBase(name, width, height) {
    init();
  }

  WindowWin32::~WindowWin32() {
  }

  int WindowWin32::run() {
    // Create separate rendering thread
    std::thread renderThread([this]() { renderingThread(); });

    // Message loop
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

  std::vector<std::string> WindowWin32::getRequiredVulkanInstanceExtensions() {
    std::vector<std::string> extensions = {
      "VK_KHR_surface",
      "VK_KHR_win32_surface"
    };

    return extensions;
  }

  VkResult WindowWin32::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = m_Handle;
    createInfo.hinstance = GetModuleHandle(0);

    return vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, surface);
  }

  int WindowWin32::init() {
    m_Instance = GetModuleHandle(0);
    m_Icon = LoadIcon(m_Instance, MAKEINTRESOURCE(IDI_APP_ICON));
    m_IconSmall = LoadIcon(m_Instance, MAKEINTRESOURCE(IDI_APP_ICON));

    // Convert string name to wide string
    std::wstring wName = Win32Utilities::stringToWideString(m_Name);

    // 1. Setup window class attributes
    WNDCLASSEX wcex;
    ZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize        = sizeof(wcex);              // WNDCLASSEX size in bytes
    wcex.style         = CS_OWNDC | CS_DBLCLKS;     // Window class styles
    wcex.lpszClassName = wName.c_str();             // Window class name
    wcex.hbrBackground = NULL;                      // Window background brush color
    wcex.hCursor       = LoadIcon(NULL, IDC_ARROW); // Window cursor
    wcex.lpfnWndProc   = WindowProc;                // Window procedure (message handler)
    wcex.hInstance     = m_Instance;                // Window application instance
    wcex.hIcon         = m_Icon;                    // Window application icon
    wcex.hIconSm       = m_IconSmall;               // Window application small icon

    // 2. Register window and ensure registration success
    if (!RegisterClassEx(&wcex)) {
      std::cout << "WIN32 ERROR: Could not create window!\n";
      return 0;
    }

    // 3. Setup window initialization attributes and create window
    POINT screenCenter = Win32Utilities::getScreenCenter();
    m_Handle = CreateWindowEx(
      0,                             // Window extended styles
      wcex.lpszClassName,            // Window class name
      wName.c_str(),                 // Window title
      WS_OVERLAPPEDWINDOW,           // Window style
      screenCenter.x - m_Width / 2,  // Window X position
      screenCenter.y - m_Height / 2, // Window Y position
      m_Width,                       // Window width
      m_Height,                      // Window height
      NULL,                          // Window parent
      NULL,                          // Window menu
      m_Instance,                    // Window instance
      this                           // Window creation parameters
    );

    // Validate window
    if (!m_Handle) { return 0; }

    return 1;
  }

  void WindowWin32::createGraphicsContext() {
    m_HDC = GetDC(m_Handle);
    m_GraphicsDevice = std::make_shared<GraphicsDevice_Vulkan>(*this);
    fz::GetDevice() = m_GraphicsDevice.get();

    m_Renderer = std::make_shared<Renderer>(*this);
    m_Renderer->setClearColor(m_BackgroundColor);
  }

  void WindowWin32::renderingThread() {
    bool firstPaint = true;
    createGraphicsContext();


    // TODO: Move to onCreate
    VkRenderPass renderPass = m_Renderer->getSwapChainRenderPass();
    std::vector<VkDescriptorSetLayout> setLayouts;
    m_UIRenderSystem = std::make_unique<UIRenderSystem>(*m_GraphicsDevice, renderPass, setLayouts);

    
    onCreate();
    Window::onCreate();

    auto lastTime = std::chrono::high_resolution_clock::now();

    // TODO: Make frame timing consistent when resizing window
    while (!m_ShouldClose.load(std::memory_order_relaxed)) {

      m_FrameDone.store(false, std::memory_order_relaxed);

      if (m_ShouldRender.load(std::memory_order_relaxed)) {
        auto newTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::seconds::period>(
          newTime - lastTime).count();
        lastTime = newTime;

        if (auto commandBuffer = m_Renderer->beginFrame()) {
          int frameIndex = m_Renderer->getFrameIndex();

          // Update
          FrameInfo frameInfo{};
          frameInfo.frameIndex = frameIndex;
          frameInfo.frameTime = dt;
          frameInfo.commandBuffer = commandBuffer;
          frameInfo.windowWidth = (float)m_Renderer->getSwapChainWidth();
          frameInfo.windowHeight = (float)m_Renderer->getSwapChainHeight();

          Window::onUpdate(dt);
          onUpdate(dt);
          m_UIRenderSystem->onUpdate(frameInfo);
          // ...

          // Render
          m_Renderer->beginSwapChainRenderPass(commandBuffer);
          m_UIRenderSystem->onRender(frameInfo);
          m_Renderer->endSwapChainRenderPass(commandBuffer);
          
          m_Renderer->endFrame();
        }

        m_FrameDone.store(true, std::memory_order_relaxed);

        if (firstPaint) {
          ShowWindow(m_Handle, SW_SHOW);
          UpdateWindow(m_Handle);
          firstPaint = false;
        }
      }
    }

    vkDeviceWaitIdle(m_GraphicsDevice->getDevice());
  }

  UIEvent WindowWin32::createMouseEvent(unsigned int message, uint64_t wParam, int64_t lParam)
  {
    UIEvent event = UIEvent(UIEventType::MouseMove);
    MouseEventData& mouse = event.getMouseData();

    // Acquire mouse position
    int x = GET_X_LPARAM(lParam);
    int y = GET_Y_LPARAM(lParam);

    mouse.position = { x, y };
    std::cout << "mouse pos: " << x << ", " << y << std::endl;

    // Acquire mouse button states
    mouse.downButtons.leftButton = (GET_KEYSTATE_WPARAM(wParam) & MK_LBUTTON) > 0;
    mouse.downButtons.middleButton = (GET_KEYSTATE_WPARAM(wParam) & MK_MBUTTON) > 0;
    mouse.downButtons.rightButton = (GET_KEYSTATE_WPARAM(wParam) & MK_RBUTTON) > 0;

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
    }

    bool buttonPressed = mouse.downButtons.leftButton || mouse.downButtons.middleButton || mouse.downButtons.rightButton;

    // Set event types accordingly
    switch (message) {
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    case WM_XBUTTONUP:
      event.setType(UIEventType::MouseUp);
      event.getMouseData().clickCount = 0;

      if (!buttonPressed) {
        ReleaseCapture();
      }
      break;
    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    case WM_XBUTTONDBLCLK:
      event.setType(UIEventType::MouseDown);
      event.getMouseData().clickCount = 2;
      break;
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_XBUTTONDOWN:
      event.setType(UIEventType::MouseDown);
      event.getMouseData().clickCount = 1;
      SetCapture(m_Handle);
      break;
    case WM_MOUSELEAVE:
    case WM_NCMOUSELEAVE:
      {
      event.setType(UIEventType::MouseExitWindow);
      m_TrackingMouseLeave = false;
      }
      break;
    case WM_MOUSEMOVE:
      event.setType(buttonPressed ? UIEventType::MouseDrag : UIEventType::MouseMove);

      if (m_MouseButtonEvent.getType() != UIEventType::None) {
        /*r.mouse().down_position = mouse_button_event.mouse().down_position;
        r.mouse().click_count = mouse_button_event.mouse().click_count;*/
      }
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

  // ------ Event functions ------
  void WindowWin32::onCreate() {
    auto minimizeIcon = Texture2D::create("assets/icons/minimize.png");
    auto maximizeIcon = Texture2D::create("assets/icons/maximize.png");
    auto closeIcon = Texture2D::create("assets/icons/close.png");

    m_MinimizeButton = &makeElement<UIIconButton>("Minimize", glm::vec2(m_Width - 90, 0), 30, 30, minimizeIcon);
    m_MinimizeButton->setOnClick([this]() { SendMessage(m_Handle, WM_SYSCOMMAND, SC_MINIMIZE, 0); });

    m_MaximizeButton = &makeElement<UIIconButton>("Maximize", glm::vec2(m_Width - 60, 0), 30, 30, maximizeIcon);
    m_MaximizeButton->setOnClick([this]() {
      WINDOWPLACEMENT wp{};
      GetWindowPlacement(m_Handle, &wp);
      ShowWindow(m_Handle, wp.showCmd == SW_MAXIMIZE ? SW_RESTORE : SW_MAXIMIZE);
    });

    m_CloseButton = &makeElement<UIIconButton>("Close", glm::vec2(m_Width - 30, 0), 30, 30, closeIcon);
    m_CloseButton->setBackgroundHoverColor({ 200, 0, 0 });
    m_CloseButton->setBackgroundClickColor({ 128, 0, 0 });
    m_CloseButton->setOnClick([this]() { SendMessage(m_Handle, WM_CLOSE, 0, 0); });
  }

  void WindowWin32::onUpdate(float dt) {
    m_MinimizeButton->setPosition({ m_Width - 90, 0 });
    m_MaximizeButton->setPosition({ m_Width - 60, 0 });
    m_CloseButton->setPosition({ m_Width - 30, 0 });
  }

  void WindowWin32::processEvent(const UIEvent& event)
  {
    WindowBase::processEvent(event);

    switch (event.getType()) {
    case UIEventType::MouseDown:
      {
        auto& mouse = event.getMouseData();
        if (isCursorInTitleBar(mouse.position.x, mouse.position.y)) {
          if (mouse.causeButtons.leftButton) {
            if (mouse.clickCount == 1) {
              ReleaseCapture();
              SendMessage(m_Handle, WM_SYSCOMMAND, 0xf012, 0); // Hack: Use of undocumented flag "SC_DRAGMOVE"
            }
            else if (mouse.clickCount == 2) { // caption bar double click)
              WINDOWPLACEMENT wp{};
              GetWindowPlacement(m_Handle, &wp);
              ShowWindow(m_Handle, wp.showCmd == SW_MAXIMIZE ? SW_RESTORE : SW_MAXIMIZE);
            }
          }
        }
      }
      break;
    }
  }

  bool WindowWin32::isCursorInTitleBar(int x, int y)
  {
    return x < m_Width - 90 && y < 29;
  }

  bool WindowWin32::isCursorOnBorder(int x, int y)
  {
    RECT windowRect;
    GetWindowRect(m_Handle, &windowRect);

    return (x <= windowRect.right && x > windowRect.right - 8) ||
           (x >= windowRect.left && x < windowRect.left + 8) ||
           (y >= windowRect.top && y < windowRect.top + 8) ||
           (y <= windowRect.bottom && y > windowRect.bottom + 8);
  }

  // Hit test the frame for resizing and moving.
  LRESULT WindowWin32::HitTestNCA(HWND hWnd, WPARAM wParam, LPARAM lParam)
  {
    // Get the point coordinates for the hit test
    POINT ptMouse = { LOWORD(lParam), HIWORD(lParam) };

    // Get the window rectangle
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
      //return HTCLOSE;
    }
    else if (uRow == 0 && ptMouse.x >= rcWindow.right - 60 && ptMouse.x < rcWindow.right - 30) {
      //return HTMAXBUTTON;
    }
    else if (uRow == 0 && ptMouse.x >= rcWindow.right - 90 && ptMouse.x < rcWindow.right - 60) {
      //return HTMINBUTTON;
    }

    if (ptMouse.x >= rcWindow.left && ptMouse.x < rcWindow.right &&
        ptMouse.y >= rcWindow.top && ptMouse.y < rcWindow.bottom &&
        uRow == 1 && uCol == 1) {
          return HTCLIENT;
    }

    // Hit test (HTTOPLEFT, ... HTBOTTOMRIGHT)
    LRESULT hitTests[3][3] =
    {
      { HTTOPLEFT,    fOnResizeBorder ? HTTOP : HTCLIENT,    HTTOPRIGHT },
      { HTLEFT,       HTNOWHERE,     HTRIGHT },
      { HTBOTTOMLEFT, HTBOTTOM, HTBOTTOMRIGHT },
    };

    return hitTests[uRow][uCol];
}

  LRESULT WindowWin32::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;
    WindowWin32* window = nullptr;
    window = reinterpret_cast<WindowWin32*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    if (message == WM_NCCREATE) {
      // Set window pointer on create
      CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
      WindowWin32* window = reinterpret_cast<WindowWin32*>(pCreate->lpCreateParams);
      SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)window);
      window = reinterpret_cast<WindowWin32*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
      
      RECT rcClient;
      GetWindowRect(hWnd, &rcClient);

      // Inform the application of the frame change
      SetWindowPos(hWnd, NULL, rcClient.left, rcClient.top, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, SWP_FRAMECHANGED);

      result = DefWindowProc(hWnd, message, wParam, lParam);
    }
    else {
      bool wasHandled = false;

      switch (message) {
        case WM_ERASEBKGND:
        {
          return 1;
        }
        case WM_GETMINMAXINFO:
        {
          if (window != nullptr) {
            LPMINMAXINFO minMaxInfo = reinterpret_cast<LPMINMAXINFO>(lParam);
            minMaxInfo->ptMinTrackSize.x = window->m_MinWidth;
            minMaxInfo->ptMinTrackSize.y = window->m_MinHeight;
          }
          break;
        }
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
          if (message == WM_MOUSELEAVE) { std::cout << "Mouse leave" << std::endl; }
          window->processEvent(window->createMouseEvent(message, wParam, lParam));
          break;
        case WM_NCHITTEST:
        {
          std::cout << "Hittest: ";
          result = HitTestNCA(hWnd, wParam, lParam);
          std::cout << result << std::endl;
          wasHandled = true;
          break;
        }
        // Handling this message allows us to extend the client area of the window
        // into the title bar region. Which effectively makes the whole window
        // region accessible for drawing, including the title bar.
        case WM_NCCALCSIZE:
        {
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

          return 0;
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
          window->m_Width = LOWORD(lParam);
          window->m_Height = HIWORD(lParam);
          
          
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

  void WindowWin32::setMinimumSize(uint32_t width, uint32_t height)
  {
    UINT dpi = GetDpiForWindow(m_Handle);
    int minWidth = GetSystemMetricsForDpi(SM_CXMINTRACK, dpi);
    int minHeight = GetSystemMetricsForDpi(SM_CYMINTRACK, dpi);

    assert(width >= minWidth && "ASSERTION FAILED: Requested minimum width is too small!");
    assert(height >= minHeight && "ASSERTION FAILED: Requested minimum height is too small!");

    m_MinWidth = width;
    m_MinHeight = height;
  }

}

#ifndef FZ_WIN32_WINDOW_HEADER
#define FZ_WIN32_WINDOW_HEADER

// std
#include <atomic>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <mutex>
#include <memory>
#include <condition_variable>

// FZUI
#include "fzui/core.hpp"

// Windows
#include <windows.h>

namespace fz {
  class FZ_API WindowWin32 : public WindowBase {
    public:
      WindowWin32(const std::string& name, int width, int height);
      virtual ~WindowWin32();

      int run();
      virtual std::vector<std::string> getRequiredVulkanInstanceExtensions() override;
      virtual VkResult createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) override;

      // Event functions
      virtual void onCreate();
      virtual void onResize() {};
      virtual void onUpdate(float dt) override;
      virtual void onDestroy() {};

      // Getters
      virtual int getWidth() const;
      virtual int getHeight() const;
      virtual HWND getHandle() const;

    private:
      int init();
      void createGraphicsContext();
      void renderingThread();

      static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
      static LRESULT HitTestNCA(HWND hWnd, WPARAM wParam, LPARAM lParam);

      // Rendering

      HDC m_HDC = NULL;
      int m_Vsync = 0;

      std::atomic<bool> m_ShouldClose = false;
      std::atomic<bool> m_ShouldRender = true;
      std::atomic<bool> m_Resizing = false;
      std::atomic<bool> m_FrameDone = false;
      std::atomic<bool> m_SplashScreenActive = true;
      std::atomic<bool> m_FirstPaint = true; // false once the first frame has been rendered

      HINSTANCE m_Instance = NULL;
      HWND m_Handle = NULL;
      HICON m_Icon = NULL;
      HICON m_IconSmall = NULL;
      bool m_MinimizeButtonDown = false;
      bool m_MaximizeButtonDown = false;
      bool m_CloseButtonDown = false;
  };

  typedef WindowWin32 Window;
}
#endif

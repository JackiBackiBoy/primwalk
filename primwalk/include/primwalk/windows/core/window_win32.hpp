#ifndef PW_WIN32_WINDOW_HEADER
#define PW_WIN32_WINDOW_HEADER

// std
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include <thread>

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/ui/uiEvent.hpp"

// Windows
#include <windows.h>

namespace pw {
  // Forward declarations
  class UIIconButton;

  class PW_API WindowWin32 : public WindowBase {
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

      virtual void processEvent(const UIEvent& event) override;
      virtual bool isCursorInTitleBar(int x, int y) override;
      virtual bool isCursorOnBorder(int x, int y) override;

      // Getters
      virtual HWND getHandle() const;

      // Setters
      virtual void setMinimumSize(uint32_t width, uint32_t height) override;
      virtual void setCursor(MouseCursor cursor) override;

    private:
      int init();
      void createGraphicsContext();
      void renderingThread();
      UIEvent createMouseEvent(unsigned int message, uint64_t wParam, int64_t lParam);

      static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
      static LRESULT HitTestNCA(HWND hWnd, WPARAM wParam, LPARAM lParam);

      // Rendering
      HDC m_HDC = NULL;
      std::atomic<bool> m_ShouldClose = false;
      std::atomic<bool> m_ShouldRender = true;
      std::atomic<bool> m_Resizing = false;
      std::atomic<bool> m_FrameDone = false;
      std::atomic<bool> m_FirstPaint = true; // false once the first frame has been rendered

      HINSTANCE m_Instance = NULL;
      HWND m_Handle = NULL;
      HICON m_Icon = NULL;
      HICON m_IconSmall = NULL;
      bool m_TrackingMouseLeave = false;
      UIIconButton* m_MinimizeButton = nullptr;
      UIIconButton* m_MaximizeButton = nullptr;
      UIIconButton* m_CloseButton = nullptr;
      UIEvent m_MouseButtonEvent = { UIEventType::None };
  };

  typedef WindowWin32 Window;
}
#endif

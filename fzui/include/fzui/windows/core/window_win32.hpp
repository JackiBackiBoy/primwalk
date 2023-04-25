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
#include <condition_variable>

// FZUI
#include "fzui/core.hpp"
#include "fzui/uiContainer.hpp"
#include "fzui/uiElement.hpp"
#include "fzui/rendering/renderer2d.hpp"
#include "fzui/data/texture.hpp"

// Windows
#include <windows.h>

namespace fz {
  class FZ_API WindowWin32 : public WindowBase {
    public:
      WindowWin32(const std::string& name, const int& width, const int& height, WindowWin32* parent = nullptr);
      virtual ~WindowWin32();

      int run();

      // Event functions
      virtual void onCreate();
      virtual void onResize() {};
      virtual void onUpdate(float dt) override;
      virtual void onRender(float dt) override;
      virtual void onDestroy() {};

      // UI
      virtual void addElement(UIElement* elem);
      virtual void addContainer(UIContainer* container);

      // Getters
      virtual int getWidth() const;
      virtual int getHeight() const;

    private:
      int init();
      void createGraphicsContext();
      void renderingThread();

      static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
      static LRESULT HitTestNCA(HWND hWnd, WPARAM wParam, LPARAM lParam);

      // Rendering
      Renderer2D* m_Renderer2D = nullptr;

      HDC m_HDC = NULL;
      int m_Vsync = 0;

      std::string m_Name = "";
      std::atomic<int> m_Width = 0;
      std::atomic<int> m_Height = 0;
      std::atomic<bool> m_ShouldClose = false;
      std::atomic<bool> m_ShouldRender = true;
      std::atomic<bool> m_Resizing = false;
      std::atomic<bool> m_FrameDone = false;
      std::atomic<bool> m_SplashScreenActive = true;

      Texture m_MinimizeIcon;
      Texture m_MaximizeIcon;
      Texture m_CloseIcon;
      HGLRC hglrc = NULL;
      HINSTANCE m_Instance = NULL;
      HWND m_Handle = NULL;
      HWND m_MenuHandle = NULL;
      HICON m_Icon = NULL;
      HICON m_IconSmall = NULL;
      bool m_MinimizeButtonDown = false;
      bool m_MaximizeButtonDown = false;
      bool m_CloseButtonDown = false;
      std::vector<UIElement*> m_UIElements;
      std::vector<UIContainer*> m_UIContainers;
  };
}
#endif

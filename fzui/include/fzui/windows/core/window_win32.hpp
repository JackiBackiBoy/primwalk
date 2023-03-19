#ifndef FZ_WIN32_WINDOW_HEADER
#define FZ_WIN32_WINDOW_HEADER

// std
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

// FZUI
#include "fzui/core.hpp"
#include "fzui/windows/ui/uiElement.hpp"
#include "fzui/windows/rendering/renderer2d.hpp"

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
      virtual void onUpdate(const float& dt) override;
      virtual void onRender(const float& dt) override;
      virtual void onDestroy() {};

      // UI
      virtual void addElement(UIElement* elem);

      // Getters
      virtual int getWidth() const;
      virtual int getHeight() const;

    private:
      int init();
      void createGraphicsContext();

      static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
      static LRESULT HitTestNCA(HWND hWnd, WPARAM wParam, LPARAM lParam);

      // Rendering
      Renderer2D* m_Renderer2D = nullptr;
      Texture m_MinimizeIcon{};
      Texture m_MaximizeIcon{};
      Texture m_CloseIcon{};

      HDC m_HDC = NULL;
      int m_Vsync = 0;
      HBRUSH m_BackgroundBrush = NULL;
      std::string m_Name = "";
      int m_Width = 0;
      int m_Height = 0;
      HINSTANCE m_Instance = NULL;
      HWND m_Handle = NULL;
      HWND m_MenuHandle = NULL;
      HICON m_Icon = NULL;
      HICON m_IconSmall = NULL;
      bool m_MaximizeButtonDown = false;
      std::vector<UIElement*> m_UIElements;
  };
}
#endif

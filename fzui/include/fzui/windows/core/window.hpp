#pragma once

// std
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

// FZUI
#include "fzui/core.hpp"
#include "fzui/windows/ui/uiElement.hpp"
#include "fzui/windows/rendering/renderer2d.hpp"
#include "fzui/windows/data/shader.hpp"

// Windows
#include <windows.h>

namespace fz {
  class FZ_API Window {
    public:
      Window(const std::wstring& name, const int& width, const int& height, Window* parent = nullptr);
      ~Window();

      int run();

      // Event functions
      virtual void onCreate(HWND hWnd);
      virtual void onResize() {};
      virtual void onRender();
      virtual void onDestroy() {};

    private:
      int init();
      void createGraphicsContext();

      static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

      // Rendering
      Renderer2D* m_Renderer2D = nullptr;

      Shader shader;
      HDC m_HDC = NULL;
      int m_Vsync = 0;
      HBRUSH m_BackgroundBrush = NULL;
      std::wstring m_Name = L"";
      int m_Width = 0;
      int m_Height = 0;
      HINSTANCE m_Instance = NULL;
      HWND m_Handle = NULL;
      HWND m_MenuHandle = NULL;
      HICON m_Icon = NULL;
      HICON m_IconSmall = NULL;
      std::vector<UIElement*> m_UIElements;
  };
}

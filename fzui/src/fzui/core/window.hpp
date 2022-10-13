#pragma once

// std
#include <string>
#include <unordered_map>

#include <windows.h>
#include "fzui/core/core.hpp"
#include "fzui/ui/win32/win32UiElement.hpp"

namespace fz {
  class FZ_API Window {
    public:
      Window(const std::wstring& name, const int& width, const int& height);
      ~Window();

      int run(HINSTANCE hInstance);

      // Event functions
      virtual void onCreate(HWND hWnd);
      virtual void onResize() {};
      virtual void onPaint() {};
      virtual void onDestroy() {};

      inline Win32UiElement* getUiElement(const int& id) { return m_UiElements[id]; }

      void addUiElement(Win32UiElement* element);
      HBRUSH backgroundBrush;

    private:
      int init(HINSTANCE hInstance);
      static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

      std::wstring m_Name;
      int m_Width;
      int m_Height;
      HINSTANCE m_Instance;
      HWND m_Handle;
      int m_LastID = 100;
      std::unordered_map<int, HWND> m_UiHandles;
      std::unordered_map<int, Win32UiElement*> m_UiElements;
  };
}

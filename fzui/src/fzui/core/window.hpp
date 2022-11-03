#pragma once

// std
#include <string>
#include <unordered_map>

#include <windows.h>
#include "fzui/core/core.hpp"
#include "fzui/ui/win32/win32UiElement.hpp"
#include "fzui/ui/win32/win32MenuItem.hpp"
#include "fzui/ui/windowInfo.hpp"

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

      // Getters
      inline Win32UiElement* getUiElement(const int& id) { return m_UiElements[id]; }
      inline HWND getUiHandle(const int& id) { return m_UiHandles[id]; }
      inline WindowInfo getWindowInfo() const { return m_WindowInfo; }
      inline std::wstring getTitle() const { return m_Name; }

      // Setters
      inline void setWindowInfo(const WindowInfo& info) { m_WindowInfo = info; }

      void addUiElement(Win32UiElement* element);
      void addMenuItem(Win32MenuItem* menuItem);

      HBRUSH backgroundBrush;

    private:
      int init(HINSTANCE hInstance);

      static LRESULT CALLBACK TrackerProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubClass, DWORD_PTR dwRefData);
      static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
      static LRESULT CALLBACK MenuProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

      std::wstring m_Name;
      int m_Width;
      int m_Height;
      HINSTANCE m_Instance;
      HWND m_Handle;
      HWND m_MenuHandle;
      HICON m_Icon;
      HICON m_IconSmall;
      int m_LastID = 100;

      WindowInfo m_WindowInfo;

      std::unordered_map<int, HWND> m_UiHandles;
      std::unordered_map<int, Win32UiElement*> m_UiElements;
  };
}

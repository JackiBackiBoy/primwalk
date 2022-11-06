#pragma once

// std
#include <string>
#include <unordered_map>
#include <vector>

// FZUI
#include "fzui/core/core.hpp"
#include "fzui/ui/win32/win32UiElement.hpp"
#include "fzui/ui/win32/win32MenuItem.hpp"
#include "fzui/ui/windowInfo.hpp"

// Windows
#include <windows.h>

namespace fz {
  class FZ_API Window {
    public:
      Window(const std::wstring& name, const int& width, const int& height, Window* parent = nullptr);
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
      int getHeight() const;
      WindowInfo getWindowInfo() const;
      HWND getWindowHandle() const;
      HBRUSH getBackgroundBrush() const;

      // Setters
      void setWindowInfo(const WindowInfo& windowInfo);
      void setPosition(const int& x, const int& y);
      void setBackground(const Color& color);

      template <typename T>
      T* addElement() {
        m_UiElements.insert({ m_LastID, new T });

        return static_cast<T*>(m_UiElements.find(m_LastID++)->second);
      }

      void addMenuItem(Win32MenuItem* menuItem);
      void addSubWindow(Window* subWindow);
      bool hasParent() const;

    private:
      int init(HINSTANCE hInstance);

      static LRESULT CALLBACK TrackerProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubClass, DWORD_PTR dwRefData);
      static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
      static LRESULT CALLBACK MenuProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

      Window* m_Parent = nullptr;
      std::vector<Window*> m_SubWindows;

      std::wstring m_Name = L"";
      int m_Width = 0;
      int m_Height = 0;
      int m_PositionX = FZUI_DONTCARE;
      int m_PositionY = FZUI_DONTCARE;
      HINSTANCE m_Instance = NULL;
      HWND m_Handle = NULL;
      HWND m_MenuHandle = NULL;
      HICON m_Icon = NULL;
      HICON m_IconSmall = NULL;
      int m_LastID = 100;
      WindowInfo m_WindowInfo = {};
      Color m_BackgroundColor = UiStyle::darkBackground;
      HBRUSH m_BackgroundBrush = NULL;

      std::unordered_map<int, HWND> m_UiHandles;
      std::unordered_map<int, Win32UiElement*> m_UiElements;
  };
}

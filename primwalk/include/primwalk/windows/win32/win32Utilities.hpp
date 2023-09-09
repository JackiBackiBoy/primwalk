#pragma once

// std
#include <string>
#include <windows.h>

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/color.hpp"

namespace pw {
  class PW_API Win32Utilities {
    public:
      Win32Utilities(const Win32Utilities&) = delete;
      Win32Utilities(Win32Utilities&&) = delete;
      Win32Utilities& operator=(const Win32Utilities&) = delete;
      Win32Utilities& operator=(Win32Utilities&&) = delete;

      static Win32Utilities& Instance();

      static SIZE calcReqCheckBoxSize(const std::wstring& text, HFONT font, HWND hWnd);
      static SIZE calcReqLabelSize(const std::wstring& text, HFONT font, HWND hWnd);
      static SIZE calcReqButtonSize(const std::wstring& text, const int& marginX,
                                    const int& marginY, HFONT font, HWND hWnd);

      static COLORREF getColorRef(const Color& color);
      static RECT getRelativeClientRect(HWND child, HWND base);
      static std::wstring stringToWideString(const std::string& string);
      static POINT getScreenCenter();

    private:
      Win32Utilities() {};
      ~Win32Utilities() {};
  };
}


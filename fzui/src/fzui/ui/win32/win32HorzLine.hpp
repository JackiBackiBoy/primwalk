#pragma once

// std
#include <string>

#include "fzui/core/core.hpp"
#include "fzui/ui/win32/win32UiElement.hpp"

namespace fz {
  class FZ_API Win32HorzLine : public Win32UiElement {
    public:
      Win32HorzLine(const int& x, const int& y, const int& width, HWND hWnd,
          const std::string& fontName = "Segoe UI", const int& fontSize = 17,
          const int& fontFlags = 0)
        : Win32UiElement(L"", x, y, hWnd, fontName, fontSize, fontFlags),
        m_Width{width} {};

      CREATESTRUCT getCreateStruct();

    private:
      int m_Width;
  };
}

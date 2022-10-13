#pragma once

// std
#include <string>

#include "fzui/core/core.hpp"
#include "fzui/ui/win32/win32UiElement.hpp"

namespace fz {
  class FZ_API Win32CheckBox : public Win32UiElement {
    public:
      Win32CheckBox(const std::wstring& text, const int& x, const int& y, HWND hWnd,
          const std::string& fontName = "Segoe UI", const int& fontSize = 17,
          const int& fontFlags = 0)
        : Win32UiElement(text, x, y, hWnd, fontName, fontSize, fontFlags) {};

      CREATESTRUCT getCreateStruct();

    private:
  };
}

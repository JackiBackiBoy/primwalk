#pragma once

// std
#include <string>

#include <windows.h>
#include "fzui/core/core.hpp"
#include "fzui/color.hpp"

namespace fz {
  SIZE FZ_API calcReqCheckBoxSize(const std::wstring& text, HFONT font, HWND hWnd);
  SIZE FZ_API calcReqLabelSize(const std::wstring& text, HFONT font, HWND hWnd);
  COLORREF FZ_API getColorRef(const Color& color);
  RECT FZ_API getRelativeClientRect(HWND child, HWND base);
}

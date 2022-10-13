#pragma once

// std
#include <string>

#include <windows.h>
#include "fzui/core/core.hpp"

namespace fz {
  SIZE FZ_API calcReqCheckBoxSize(const std::wstring& text, HFONT font, HWND hWnd);
  SIZE FZ_API calcReqLabelSize(const std::wstring& text, HFONT font, HWND hWnd);
}

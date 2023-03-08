// FZUI
#include "fzui/mouse.hpp"

// Windows
#include <windows.h>

namespace fz {
  MouseWin32& MouseWin32::Instance() {
    static MouseWin32 instance;
    return instance;
  }

  glm::vec2 MouseWin32::getRelativePos() {
    return m_RelativePos;
  }

  glm::vec2 MouseWin32::getAbsolutePos() {
    POINT pos = { 0, 0 };
    GetCursorPos(&pos);

    return { (float)pos.x, (float)pos.y };
  }
}
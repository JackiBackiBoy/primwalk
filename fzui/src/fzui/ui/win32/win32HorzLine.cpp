#include "fzui/ui/win32/win32HorzLine.hpp"

namespace fz {
  CREATESTRUCT Win32HorzLine::getCreateStruct() {
    CREATESTRUCT cs;
    ZeroMemory(&cs, sizeof(cs));
    cs.dwExStyle = 0;
    cs.lpszClass = L"STATIC"; // Predefined system class name
    cs.lpszName = m_Text.c_str();  // Window title
    cs.style = WS_VISIBLE | WS_CHILD | SS_ETCHEDHORZ;
    cs.x = m_X; // X position
    cs.y = m_Y; // Y position
    cs.cx = m_Width; // Width
    cs.cy = 1; // Height
    cs.hwndParent = m_ParentHandle;
    cs.hMenu = NULL; // Leave ID undefined
    cs.hInstance = (HINSTANCE)GetWindowLongPtr(m_ParentHandle, GWLP_HINSTANCE);
    cs.lpCreateParams = NULL;

    return cs;
  }

  std::string Win32HorzLine::getTypeString() const {
    return "Win32HorzLine";
  }
}

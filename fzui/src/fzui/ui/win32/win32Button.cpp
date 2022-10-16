#include "fzui/ui/win32/win32Button.hpp"
#include "fzui/utilities.hpp"

namespace fz {
  CREATESTRUCT Win32Button::getCreateStruct() {
    CREATESTRUCT cs;
    ZeroMemory(&cs, sizeof(cs));
    cs.dwExStyle = 0;
    cs.lpszClass = L"BUTTON"; // Predefined system class name
    cs.lpszName = m_Text.c_str();  // Window title
    cs.style = WS_VISIBLE | WS_CHILD | SS_OWNERDRAW;
    cs.x = m_X; // X position
    cs.y = m_Y; // Y position
    cs.cx = m_Width; // Width
    cs.cy = m_Height; // Height
    cs.hwndParent = m_ParentHandle;
    cs.hMenu = NULL; // Leave ID undefined
    cs.hInstance = (HINSTANCE)GetWindowLongPtr(m_ParentHandle, GWLP_HINSTANCE);
    cs.lpCreateParams = NULL;

    return cs;
  }
}

#include "fzui/ui/win32/win32Label.hpp"
#include "fzui/ui/win32/win32Utilities.hpp"

namespace fz {
  CREATESTRUCT Win32Label::getCreateStruct() {
    SIZE reqSize = Win32Utilities::calcReqLabelSize(m_Text, m_Font, m_ParentHandle);

    CREATESTRUCT cs;
    ZeroMemory(&cs, sizeof(cs));
    cs.dwExStyle = 0;
    cs.lpszClass = L"STATIC"; // Predefined system class name
    cs.lpszName = m_Text.c_str();  // Window title
    cs.style = WS_VISIBLE | WS_CHILD | SS_OWNERDRAW;
    cs.x = m_X; // X position
    cs.y = m_Y; // Y position
    cs.cx = reqSize.cx; // Width
    cs.cy = reqSize.cy; // Height
    cs.hwndParent = m_ParentHandle;
    cs.hMenu = NULL; // Leave ID undefined
    cs.hInstance = (HINSTANCE)GetWindowLongPtr(m_ParentHandle, GWLP_HINSTANCE);
    cs.lpCreateParams = NULL;

    return cs;
  }

  std::string Win32Label::getTypeString() const {
    return "Win32Label";
  }
}

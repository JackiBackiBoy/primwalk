#include "fzui/ui/win32/win32CheckBox.hpp"
#include "fzui/ui/win32/win32Utilities.hpp"

namespace fz {
  CREATESTRUCT Win32CheckBox::getCreateStruct() {
    SIZE reqSize = Win32Utilities::calcReqCheckBoxSize(m_Text, m_Font, m_ParentHandle);

    CREATESTRUCT cs;
    ZeroMemory(&cs, sizeof(cs));
    cs.dwExStyle = 0;
    cs.lpszClass = L"BUTTON"; // Predefined system class name
    cs.lpszName = m_Text.c_str();  // Window title
    cs.style = WS_VISIBLE | WS_CHILD | BS_CHECKBOX;
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
}

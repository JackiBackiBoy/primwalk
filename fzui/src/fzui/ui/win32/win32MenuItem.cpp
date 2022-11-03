#include "fzui/ui/win32/win32MenuItem.hpp"
#include "fzui/ui/win32/win32Button.hpp"

namespace fz {
  Win32MenuItem::Win32MenuItem(const std::wstring& text, Win32MenuItem* parent)
    : m_Text{text} {
  }

  CREATESTRUCT Win32MenuItem::getCreateStruct() {
    CREATESTRUCT cs;
    ZeroMemory(&cs, sizeof(cs));
    cs.dwExStyle = 0;
    cs.lpszClass = L"BUTTON"; // Predefined system class name
    cs.lpszName = m_Text.c_str();  // Window title
    cs.style = WS_VISIBLE | WS_CHILD | BS_OWNERDRAW;
    cs.hMenu = NULL; // Leave ID undefined
    cs.lpCreateParams = NULL;

    return cs;
  }

  void Win32MenuItem::create(HWND parent) {

  }
}

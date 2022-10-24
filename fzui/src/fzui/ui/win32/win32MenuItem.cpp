#include "fzui/ui/win32/win32MenuItem.hpp"
#include <iostream>

namespace fz {
  Win32MenuItem::Win32MenuItem(const std::wstring& text, Win32MenuItem* parent)
    : m_Text{text}, m_Parent{parent} {

      if (m_Parent != nullptr) {
        parent->m_Children.push_back(this);
      }

      m_Handle = CreateMenu();
  }

  void Win32MenuItem::create(int& id, HMENU targetMenu) {
    if (m_Children.empty()) {
      AppendMenu(m_Parent == nullptr ? targetMenu : m_Handle, MF_STRING, id++, m_Text.c_str());
    }
    else {
      for (Win32MenuItem* item : m_Children) {
        item->create(id, targetMenu);

        AppendMenu(m_Parent == nullptr ? targetMenu : m_Handle, MF_POPUP, (UINT_PTR)item->getHandle(), m_Text.c_str());
      }
    }
  }
}

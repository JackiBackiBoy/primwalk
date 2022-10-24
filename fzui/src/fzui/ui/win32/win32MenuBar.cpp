#include "fzui/ui/win32/win32MenuBar.hpp"
#include "fzui/ui/win32/win32MenuItem.hpp"
#include <iostream>

namespace fz {
  Win32MenuBar::Win32MenuBar() {
    m_MenuHandle = CreateMenu();
  }

  HMENU Win32MenuBar::getMenuHandle() {
    return m_MenuHandle;
  }

  void Win32MenuBar::addItem(Win32MenuItem* item) {
    m_MenuItems.push_back(item);
  }

  void Win32MenuBar::create(int& id) {
    for (Win32MenuItem* item : m_MenuItems) {
      item->create(id, m_MenuHandle);
    }
  }
}

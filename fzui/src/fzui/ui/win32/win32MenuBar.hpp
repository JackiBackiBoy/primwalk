#pragma once

// std
#include <vector>

#include "fzui/core/core.hpp"
#include <windows.h>

namespace fz {
  class FZ_API Win32MenuItem;

  class FZ_API Win32MenuBar {
    public:
      Win32MenuBar();

      HMENU getMenuHandle();
      void addItem(Win32MenuItem* item);
      void create(int& id);

    private:
      HMENU m_MenuHandle = NULL;
      std::vector<Win32MenuItem*> m_MenuItems;
  };
}

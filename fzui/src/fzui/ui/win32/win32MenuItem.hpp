#pragma once

// std
#include <string>

#include "fzui/core/core.hpp"
#include "fzui/ui/win32/win32MenuBar.hpp"
#include <windows.h>

namespace fz {
  class FZ_API Win32MenuItem {
    public:
      Win32MenuItem(const std::wstring& text, Win32MenuItem* parent = nullptr);

      inline std::wstring getText() const { return m_Text; }
      inline HMENU getHandle() const { return m_Handle; }

    private:
      void create(int& id, HMENU targetMenu);

      std::wstring m_Text;
      HMENU m_Handle;
      Win32MenuItem* m_Parent;
      std::vector<Win32MenuItem*> m_Children;

      friend class Win32MenuBar;
  };
}

#pragma once

// std
#include <string>
#include <vector>

#include "fzui/core/core.hpp"
#include <windows.h>

namespace fz {
  class FZ_API Win32MenuItem {
    public:
      Win32MenuItem(const std::wstring& text, Win32MenuItem* parent = nullptr);

      inline std::wstring getText() const { return m_Text; }
      CREATESTRUCT getCreateStruct();

    private:
      void create(HWND parent);

      std::wstring m_Text;
      HWND m_Handle;
      HWND m_Parent;
      std::vector<Win32MenuItem*> m_Children;
  };
}

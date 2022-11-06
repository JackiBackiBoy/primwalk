#pragma once

// std
#include <functional>
#include <string>

#include "fzui/core/core.hpp"
#include <windows.h>

namespace fz {
  class FZ_API Win32UiElement {
    public:
      Win32UiElement();
      virtual ~Win32UiElement() = default;

      inline HFONT getFont() const { return m_Font; }
      void setFont();

      // Setters
      void setPosition(const int& x, const int& y);
      void setOnClick(std::function<void()> onClick);

      virtual void onClick() final { m_OnClick(); }

      // Used for creating a ui object
      virtual CREATESTRUCT getCreateStruct() = 0;

    protected:
      std::wstring m_Text;
      int m_X, m_Y;
      std::function<void()> m_OnClick;
      HWND m_ParentHandle;
      HFONT m_Font;
  };
}

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
      std::wstring getText() const;

      // Getters
      virtual CREATESTRUCT getCreateStruct() = 0;
      virtual std::string getTypeString() const = 0;
      int getPositionX() const;

      // Setters
      virtual void setPosition(const int& x, const int& y);
      void setOnClick(std::function<void()> onClick);


      virtual void onClick() final { m_OnClick(); }

    protected:
      std::wstring m_Text;
      int m_X = 0;
      int m_Y = 0;
      std::function<void()> m_OnClick;
      HWND m_Handle = NULL;
      HWND m_ParentHandle = NULL;
      HFONT m_Font = NULL;

      friend class Window;
  };
}

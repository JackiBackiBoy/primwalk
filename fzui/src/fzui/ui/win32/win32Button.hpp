#pragma once

// std
#include <string>

#include "fzui/core/core.hpp"
#include "fzui/ui/win32/win32UiElement.hpp"
#include "fzui/color.hpp"

namespace fz {
  class FZ_API Win32Button : public Win32UiElement {
    public:
      Win32Button(const std::wstring& text, const int& x, const int& y,
          const int& width, const int& height, HWND hWnd,
          const std::string& fontName = "Segoe UI", const int& fontSize = 16,
          const int& fontFlags = 0);

      // Getters
      inline int getWidth() const { return m_Width; }
      inline int getHeight() const { return m_Height; }
      inline int getBorderThickness() const { return m_BorderThickness; }
      inline int getBorderRadius() const { return m_BorderRadius; }

      inline Color getDefaultColor() const { return m_DefaultColor; };
      inline Color getHoverColor() const { return m_HoverColor; }
      inline Color getBorderColor() const { return m_BorderColor; }

      // Setters
      void setBackgroundColor(const Color& color);
      void setBorderColor(const Color& color);
      void setTextColor(const Color& color);

      CREATESTRUCT getCreateStruct();

    private:
      int m_Width;
      int m_Height;
      int m_BorderRadius;
      int m_BorderThickness;

      Color m_DefaultColor;
      Color m_HoverColor;
      Color m_BorderColor;
      Color m_TextColor;
  };
}

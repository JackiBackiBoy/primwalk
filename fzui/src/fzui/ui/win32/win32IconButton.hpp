#pragma once

// FZUI
#include "fzui/core/core.hpp"
#include "fzui/ui/win32/win32UiElement.hpp"
#include "fzui/color.hpp"

namespace fz {
  class FZ_API Win32IconButton : public Win32UiElement {
    public:
      Win32IconButton();
      ~Win32IconButton();

      // Getters
      int getWidth() const;
      int getHeight() const;
      inline int getBorderThickness() const { return m_BorderThickness; }
      inline int getBorderRadius() const { return m_BorderRadius; }

      Color getDefaultColor() const;
      Color getHoverColor() const;
      Color getBorderColor() const;

      // Setters
      void setBorderColor(const Color& color);
      void setDefaultColor(const Color& color);
      void setHoverColor(const Color& color);
      void setWidth(const int& width);
      void setHeight(const int& height);
      void setIcon(const std::wstring& path);
      void setPosition(const int& x, const int& y) override;

      // Getters
      CREATESTRUCT getCreateStruct() override;
      std::string getTypeString() const override;
      HICON getIcon() const;

    private:
      int m_Width = 0;
      int m_Height = 0;
      int m_BorderRadius;
      int m_BorderThickness;

      Color m_DefaultColor;
      Color m_HoverColor;
      Color m_BorderColor;
      Color m_TextColor;

      // Win32
      HICON m_Icon = NULL;
  };
}

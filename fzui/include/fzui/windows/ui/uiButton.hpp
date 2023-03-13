#pragma once

// std
#include <string>

// FZUI
#include "fzui/core.hpp"
#include "fzui/windows/ui/uiElement.hpp"
#include "fzui/windows/color.hpp"

namespace fz {
  class FZ_API UIButton : public UIElement {
    public:
      UIButton(const std::string& text, const glm::vec2& pos,
               const int& width, const int& height);
      virtual ~UIButton() = default;

      virtual void update(const float& dt) override;
      virtual void draw(Renderer2D* renderer) override;

      // Setters
      void setTextColor(const Color& color);
      void setBackgroundColor(const Color& color);
      void setHoverColor(const Color& hoverColor);
      void setHoverTransition(const float& duration);

    private:
      std::string m_Text;
      int m_Width;
      int m_Height;
      float m_HoverTransition;

      // Colors
      Color m_TextColor;
      Color m_BackgroundColor;
      Color m_HoverColor;
      Color m_DisplayColor;

      // Flags
      bool m_IsHovered = false;
      float m_Timer = 0.0f;
  };
}

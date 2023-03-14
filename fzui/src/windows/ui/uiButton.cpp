// FZUI
#include "fzui/windows/ui/uiButton.hpp"
#include "fzui/mouse.hpp"
#include "fzui/windows/math/math.hpp"

// std
#include <iostream>

namespace fz {
  UIButton::UIButton(const std::string& text, const glm::vec2& pos,
                     const int& width, const int& height) :
    UIElement(pos), m_Text(text), m_Width(width), m_Height(height) {
      m_TextColor = { 255, 255, 255 };
      m_BackgroundColor = { 255, 255, 255 };
      m_HoverColor = { 200, 200, 200 };
      m_DisplayColor = m_BackgroundColor;
      m_HoverTransition = 1.0f; // seconds
  }

  void UIButton::update(const float& dt) {
    glm::vec2 mousePos = Mouse::Instance().getRelativePos();

    // Collision detection
    if (mousePos.x >= m_Position.x && mousePos.x <= m_Position.x + m_Width &&
        mousePos.y >= m_Position.y && mousePos.y <= m_Position.y + m_Height) {
      m_IsHovered = true;
      m_Timer = std::clamp(m_Timer + dt, 0.0f, m_HoverTransition);
    }
    else {
      m_IsHovered = false;
      m_Timer = std::clamp(m_Timer - dt, 0.0f, m_HoverTransition);
    }

    // Hover and unhover animation
    float percentage = m_Timer / m_HoverTransition;
    m_DisplayColor.r = Math::lerp(m_BackgroundColor.r, m_HoverColor.r, percentage);
    m_DisplayColor.g = Math::lerp(m_BackgroundColor.g, m_HoverColor.g, percentage);
    m_DisplayColor.b = Math::lerp(m_BackgroundColor.b, m_HoverColor.b, percentage);
  }

  void UIButton::draw(Renderer2D* renderer) {
    renderer->drawRect(m_Width, m_Height, m_Position, m_DisplayColor);
    renderer->drawText(m_Text, m_Position, 12, { 0, 0, 0 });
  }

  // Setters
  void UIButton::setTextColor(const Color& color) {
    m_TextColor = color;
  }

  void UIButton::setBackgroundColor(const Color& color) {
    m_BackgroundColor = color;
  }

  void UIButton::setHoverColor(const Color& color) {
    m_HoverColor = color;
  }

  void UIButton::setHoverTransition(const float& duration) {
    m_HoverTransition = duration;
  }
}

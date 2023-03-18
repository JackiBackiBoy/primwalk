// FZUI
#include "fzui/windows/ui/uiImageButton.hpp"
#include "fzui/mouse.hpp"
#include "fzui/windows/math/math.hpp"

// std
#include <algorithm>

namespace fz {
  UIImageButton::UIImageButton(Texture* texture, const glm::vec2& pos, const int& width, const int& height) :
    UIElement(pos), m_Texture(texture), m_Width(width), m_Height(height) {
      m_BackgroundColor = { 0, 0, 0 };
      m_HoverColor = { 128, 128, 128 };
      m_DisplayColor = { 255, 255, 255 };
  }

  void UIImageButton::update(const float& dt) {
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
    m_DisplayColor.a = Math::lerp(m_BackgroundColor.a, m_HoverColor.a, percentage);
  }

  void UIImageButton::draw(Renderer2D* renderer) {
    renderer->drawRect(m_Width, m_Height, m_Position, m_DisplayColor);
    renderer->drawRect(m_Width, m_Height, m_Position, { 255, 255, 255 }, m_Texture);
  }

  // Setters
  void UIImageButton::setBackgroundColor(const Color& color) {
    m_BackgroundColor = color;
  }

  void UIImageButton::setHoverColor(const Color& color) {
    m_HoverColor = color;
  }
}
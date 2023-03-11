// FZUI
#include "fzui/windows/ui/uiButton.hpp"
#include "fzui/mouse.hpp"

namespace fz {
  UIButton::UIButton(const std::string& text, const glm::vec2& pos,
                     const int& width, const int& height) :
    UIElement(pos), m_Text(text), m_Width(width), m_Height(height) {
      m_TextColor = { 255, 255, 255 };
      m_BackgroundColor = { 255, 255, 255 };
      m_HoverColor = { 200, 200, 200 };
      m_DisplayColor = m_BackgroundColor;
  }

  void UIButton::update(const float& dt) {
    glm::vec2 mousePos = Mouse::Instance().getRelativePos();

    // Collision detection
    if (mousePos.x >= m_Position.x && mousePos.y >= m_Position.y &&
        mousePos.x <= m_Position.x + m_Width &&
        mousePos.y <= m_Position.y + m_Height) {
          m_DisplayColor = m_HoverColor;
          return;
    }

    m_DisplayColor = m_BackgroundColor;
  }

  void UIButton::draw(Renderer2D* renderer) {
    renderer->drawRect(m_Width, m_Height, m_Position, m_DisplayColor);
    renderer->drawText(m_Text, m_Position, 15, { 0, 0, 0 });
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
}

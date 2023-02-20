#include "fzui/windows/ui/uiButton.hpp"

namespace fz {
  UIButton::UIButton(const std::string& text, const glm::vec2& pos,
                     const int& width, const int& height) :
    UIElement(pos), m_Text(text), m_Width(width), m_Height(height) {

  }

  void UIButton::draw(Renderer2D* renderer) {
    // Calculate center text position

    renderer->drawRect(m_Width, m_Height, m_Position, { 1.0f, 1.0f, 0.0f }, 0);
    renderer->drawText(m_Text, m_Position, 15, { 0.0f, 0.0f, 0.0f });
  }
}

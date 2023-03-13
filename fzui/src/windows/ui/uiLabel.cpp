// FZUI
#include "fzui/windows/ui/uiLabel.hpp"

namespace fz {
  UILabel::UILabel(const std::string& text, const glm::vec2& pos) :
    UIElement(pos), m_Text(text) {
      m_TextColor = { 255, 0, 255 };
  }

  void UILabel::update(const float& dt) {

  }

  void UILabel::draw(Renderer2D* renderer) {
    renderer->drawText(m_Text, m_Position, 26.0, m_TextColor);
  }
}
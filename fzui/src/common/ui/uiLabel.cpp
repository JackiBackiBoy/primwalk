// FZUI
#include "fzui/uiLabel.hpp"

namespace fz {
  UILabel::UILabel(const std::string& text, const glm::vec2& pos, const Color& color, const float& fontSize, FontFace* font) :
    UIElement(pos) {
      m_Text = text;
      m_TextColor = color;
      m_FontSize = fontSize;
      m_Font = font;
  }

  void UILabel::update(const float& dt) {

  }

  void UILabel::draw(Renderer2D* renderer) {
    renderer->drawText(m_Text, getAbsolutePosition(), m_FontSize, m_TextColor, m_Font);
  }
}

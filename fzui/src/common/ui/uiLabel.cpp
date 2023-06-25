#include "fzui/ui/uiLabel.hpp"

namespace fz {

  UILabel::UILabel(const std::string& text, glm::vec2 position, std::shared_ptr<Font> font) :
    UIElement(position), m_Text(text), m_Font(font)
  {
    if (m_Font != nullptr) {
      m_FontSize = m_Font->getFontSize();
    }
  }

  void UILabel::onRender(UIRenderSystem& renderer)
  {
    renderer.drawText(getAbsolutePosition(), m_Text, m_FontSize, m_TextColor, m_Font);
  }

  void UILabel::handleEvent(const UIEvent& event)
  {
    return;
  }

  Hitbox UILabel::hitboxTest(glm::vec2 position)
  {
    if (getHitbox().contains(position)) {
      return Hitbox(getAbsolutePosition(), 0, 0, this);
    }

    return Hitbox(getAbsolutePosition(), 0, 0, nullptr);
  }

  Hitbox UILabel::getHitbox()
  {
    return { getAbsolutePosition(), 0, 0, this };
  }

  void UILabel::setText(const std::string& text)
  {

  }

  void UILabel::setBackgroundColor(Color color)
  {

  }

}
#include "primwalk/ui/uiLabel.hpp"

namespace pw {

  UILabel::UILabel() :
    UIElement({ 0, 0 })
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
    if (Hitbox(getAbsolutePosition(), 0, 0, nullptr).contains(position)) {
      return Hitbox(getAbsolutePosition(), 0, 0, this);
    }

    return Hitbox(getAbsolutePosition(), 0, 0, nullptr);
  }

  void UILabel::setBackgroundColor(Color color)
  {

  }

}
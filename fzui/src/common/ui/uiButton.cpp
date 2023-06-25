#include "fzui/ui/uiButton.hpp"
#include "fzui/data/font.hpp"

namespace fz {

  void UIButton::onRender(UIRenderSystem& renderer)
  {
    glm::vec2 center(getAbsolutePosition());
    center.x += m_Width / 2 - m_Font->getTextWidth(m_Text, m_Font->getFontSize()) / 2;
    center.y += m_Height / 2 - m_Font->getMaxHeight() / 2;

    if (m_Icon != nullptr) {
      renderer.drawRect(getAbsolutePosition() - glm::vec2(30, 0), 30, 30, Color::White, 0, m_Icon);
    }
    renderer.drawRect(getAbsolutePosition(), m_Width, m_Height, m_DisplayColor, m_BorderRadius);
    renderer.drawText(center, m_Text, 0, m_TextColor, m_Font);
  }

  void UIButton::handleEvent(const UIEvent& event)
  {
    switch (event.getType()) {
    case UIEventType::MouseEnter:
      m_Hovered = true;
      m_DisplayColor = m_BackgroundHoverColor;
      break;
    case UIEventType::MouseExit:
      m_Hovered = false;
      m_Pressed = false;
      m_DisplayColor = m_BackgroundColor;
      break;
    case UIEventType::MouseDown:
      m_Pressed = true;
      m_DisplayColor = m_BackgroundClickColor;
      break;
    case UIEventType::MouseUp:
      if (m_Pressed) {
        m_Pressed = false;
        m_DisplayColor = m_BackgroundHoverColor;
      }
    }
  }

  Hitbox UIButton::hitboxTest(glm::vec2 position)
  {
    if (getHitbox().contains(position)) {
      return Hitbox(getAbsolutePosition(), m_Width, m_Height, this);
    }

    return Hitbox(getAbsolutePosition(), m_Width, m_Height, nullptr);
  }

  Hitbox UIButton::getHitbox()
  {
    return { getAbsolutePosition(), m_Width, m_Height, this };
  }

  // Setters
  void UIButton::setBackgroundColor(Color color)
  {
    m_BackgroundColor = color;

    if (!m_Hovered) {
      m_DisplayColor = m_BackgroundColor;
    }
  }

}
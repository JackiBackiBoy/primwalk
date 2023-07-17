#include "primwalk/ui/uiIconButton.hpp"

namespace pw {

  void UIIconButton::onRender(UIRenderSystem& renderer)
  {
    renderer.drawRect(getAbsolutePosition(), m_Width, m_Height, m_BackgroundDisplayColor);
    renderer.drawRect(getAbsolutePosition(), m_Width, m_Height, m_IconColor, 0, m_Icon);
  }

  void UIIconButton::handleEvent(const UIEvent& event)
  {
    switch (event.getType()) {
    case UIEventType::MouseEnter:
      m_Hovered = true;
      m_BackgroundDisplayColor = m_BackgroundHoverColor;
      break;
    case UIEventType::MouseExit:
      m_Hovered = false;
      m_Pressed = false;
      m_BackgroundDisplayColor = m_BackgroundColor;
      break;
    case UIEventType::MouseDown:
      m_Pressed = true;
      m_BackgroundDisplayColor = m_BackgroundClickColor;
      break;
    case UIEventType::MouseUp:
      if (m_Pressed) { // button has been clicked
        m_OnClick();
        m_Pressed = false;
        m_BackgroundDisplayColor = m_BackgroundHoverColor;
      }
    }
  }

  Hitbox UIIconButton::hitboxTest(glm::vec2 position)
  {
    if (getHitbox().contains(position)) {
      return Hitbox(getAbsolutePosition(), m_Width, m_Height, this);
    }

    return Hitbox(getAbsolutePosition(), m_Width, m_Height, nullptr);
  }

  Hitbox UIIconButton::getHitbox()
  {
    return Hitbox(getAbsolutePosition(), m_Width, m_Height, this);
  }

}
#include "fzui/ui/uiContainer.hpp"

namespace fz {

  void UIContainer::onRender(UIRenderSystem& renderer)
  {
    renderer.drawRect(getAbsolutePosition(), m_Width, m_Height, m_BackgroundColor, m_BorderRadius);

    // Render all descendant UI elements
    for (auto& e : m_Elements) {
      e->onRender(renderer);
    }
  }

  void UIContainer::handleEvent(const UIEvent& event)
  {
  }

  Hitbox UIContainer::hitboxTest(glm::vec2 position)
  {
    if (!getHitbox().contains(position)) {
      return Hitbox(getAbsolutePosition(), m_Width, m_Height, nullptr);
    }

    for (auto& e : m_Elements) {
      Hitbox hitbox = e->hitboxTest(position);

      if (hitbox.contains(position)) {
        return hitbox;
      }
    }

    return Hitbox(getAbsolutePosition(), m_Width, m_Height, this);
  }

  Hitbox UIContainer::getHitbox()
  {
    return Hitbox(getAbsolutePosition(), m_Width, m_Height, this);
  }

}
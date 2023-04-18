// FZUI
#include "fzui/uiElement.hpp"
#include "fzui/uiContainer.hpp"

namespace fz {
  void UIElement::setPosition(const glm::vec2& position) {
    m_Position = position;
  }

  const glm::vec2& UIElement::getPosition() const {
    return m_Position;
  }

  glm::vec2 UIElement::getAbsolutePosition() const
  {
    if (m_Container != nullptr) {
      return m_Position + m_Container->getAbsolutePosition();
    }

    return m_Position;
  }

}

#include "fzui/ui/uiElement.hpp"
#include "fzui/ui/uiContainer.hpp"

namespace fz {
  glm::vec2 UIElement::getPosition() const
  {
    return m_Position;
  }

  glm::vec2 UIElement::getAbsolutePosition() const
  {
    if (m_Container != nullptr) {
      return m_Position + m_Container->getAbsolutePosition();
    }

    return m_Position;
  }

  void UIElement::setPosition(glm::vec2 position)
  {
    m_Position = position;
  }

}
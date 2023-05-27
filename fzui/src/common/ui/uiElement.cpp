#include "fzui/uiElement.hpp"

namespace fz {

  glm::vec2 UIElement::getPosition() const
  {
    return m_Position;
  }

  void UIElement::setPosition(glm::vec2 position)
  {
    m_Position = position;
  }

}
// FZUI
#include "fzui/windows/ui/uiElement.hpp"

namespace fz {
  void UIElement::setPosition(const glm::vec2& position) {
    m_Position = position;
  }

  const glm::vec2& UIElement::getPosition() const {
    return m_Position;
  }
}

// FZUI
#include "fzui/ui/uiElement.hpp"

namespace fz {
  void UIElement::draw(Renderer2D* renderer) {
    renderer->drawQuad({ 0.0f, 200.0f }, { 0.0f, 0.0f }, { 200.0f, 200.0f }, { 200.0f, 0.0f });
  }
}

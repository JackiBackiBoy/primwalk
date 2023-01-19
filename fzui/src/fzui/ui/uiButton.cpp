#include "fzui/ui/uiButton.hpp"

namespace fz {
  UIButton::UIButton(const std::string& text, const glm::vec2& pos,
                     const int& width, const int& height) :
    UIElement(pos), m_Text(text), m_Width(width), m_Height(height) {

  }

  void UIButton::draw(Renderer2D* renderer) {
    //renderer->drawRect(m_Width, m_Height, m_Position, 
  }
}

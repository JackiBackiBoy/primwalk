#include "fzui/windows/ui/uiImage.hpp"

namespace fz {
  UIImage::UIImage(Texture* texture, const glm::vec2& pos, const int& width, const int& height) :
    UIElement(pos), m_Texture(texture), m_Width(width), m_Height(height) {
      m_Color = { 255, 255, 255 }; 
  }

  void UIImage::update(const float& dt) {

  }

  void UIImage::draw(Renderer2D* renderer) {
    renderer->drawRect(m_Width, m_Height, m_Position, m_Color, m_Texture);
  }
}
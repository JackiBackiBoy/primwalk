#include "fzui/uiImage.hpp"

namespace fz {
  UIImage::UIImage(Texture* texture, const glm::vec2& pos, const int& width, const int& height) :
    UIElement(pos), m_Texture(texture), m_Width(width), m_Height(height) {
      m_Color = { 255, 255, 255 }; 
      m_BorderRadius = 0;
  }

  void UIImage::update(const float& dt) {

  }

  void UIImage::draw(Renderer2D* renderer) {
    renderer->drawRect(m_Width, m_Height, m_Position, m_Color, m_BorderRadius, m_Texture);
  }

  // Getters
  int UIImage::getWidth() const {
    return m_Width;
  }

  int UIImage::getHeight() const {
    return m_Height;
  }

  // Setters
  void UIImage::setBorderRadius(int radius) {
    m_BorderRadius = radius;
  }
}

#include "fzui/uiButton.hpp"
#include "fzui/data/font.hpp"

namespace fz {

  void UIButton::onRender(UIRenderSystem& renderer)
  {
    renderer.drawRect(m_Position, m_Width, m_Height, m_BackgroundColor);
    renderer.drawText(m_Position + glm::vec2(5, 54), m_Text, 15, {255, 255, 255}, m_FontName);
  }

  void UIButton::setText(const std::string& text)
  {
    m_Text = text;
  }

  void UIButton::setBackgroundColor(Color color)
  {
    m_BackgroundColor = color;
  }

}
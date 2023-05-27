#include "fzui/uiButton.hpp"

namespace fz {

  void UIButton::onRender(UIRenderSystem& renderer)
  {
    renderer.drawRect(m_Position, m_Width, m_Height, m_BackgroundColor);
    renderer.drawText(m_Position, m_Text, { 255, 255, 255 });
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
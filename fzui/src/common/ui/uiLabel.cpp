#include "fzui/uiLabel.hpp"

namespace fz {

  void UILabel::onRender(UIRenderSystem& renderer)
  {
    renderer.drawText(m_Position, m_Text, 15, { 255, 255, 255 });
  }

  void UILabel::setText(const std::string& text)
  {

  }

  void UILabel::setBackgroundColor(Color color)
  {

  }

}
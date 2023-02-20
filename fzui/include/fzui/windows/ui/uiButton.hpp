#pragma once

// std
#include <string>

// FZUI
#include "fzui/core.hpp"
#include "fzui/windows/ui/uiElement.hpp"
#include "fzui/windows/color.hpp"

namespace fz {
  class FZ_API UIButton : public UIElement {
    public:
      UIButton(const std::string& text, const glm::vec2& pos,
               const int& width, const int& height);

      void draw(Renderer2D* renderer) override;

    private:
      std::string m_Text;
      int m_Width;
      int m_Height;
  };
}

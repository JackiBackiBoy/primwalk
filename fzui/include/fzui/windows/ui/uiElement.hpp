#pragma once

// FZUI
#include "fzui/core.hpp"
#include "fzui/windows/rendering/renderer2d.hpp"

// vendor
#include <glm/glm.hpp>

namespace fz {
  class FZ_API UIElement {
    public:
      UIElement(const glm::vec2& pos) : m_Position(pos) {};
      ~UIElement() {};

      virtual void draw(Renderer2D* renderer) = 0;

    protected:
      glm::vec2 m_Position;
  };
}

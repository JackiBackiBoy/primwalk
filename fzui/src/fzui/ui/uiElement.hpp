#pragma once

// FZUI
#include "fzui/core/core.hpp"
#include "fzui/rendering/renderer2d.hpp"

namespace fz {
  class FZ_API UIElement {
    public:
      UIElement() {};
      ~UIElement() {};

      virtual void draw(Renderer2D* renderer);

    private:

  };
}

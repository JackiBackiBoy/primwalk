#pragma once

// FZUI
#include "fzui/core.hpp"
#include "fzui/rendering/renderer2d.hpp"

// vendor
#include <glm/glm.hpp>

namespace fz {
  class FZ_API UIElement {
    public:
      UIElement(const glm::vec2& pos) : m_Position(pos) {};
      virtual ~UIElement() {};

      virtual void update(const float& dt) = 0;
      virtual void draw(Renderer2D* renderer) = 0;

      // Getters
      const glm::vec2& getPosition() const;

      // Setters
      void setPosition(const glm::vec2& position);

    protected:
      glm::vec2 m_Position;
  };
}

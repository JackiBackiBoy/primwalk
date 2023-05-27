#ifndef FZ_UI_ELEMENT_HEADER
#define FZ_UI_ELEMENT_HEADER

// FZUI
#include "fzui/core.hpp"
#include "fzui/rendering/systems/uiRenderSystem.hpp"

// vendor
#include <glm/glm.hpp>

namespace fz {
  class FZ_API UIElement {
    public:
      UIElement(glm::vec2 position) : m_Position{position} {};
      virtual ~UIElement() {};

      virtual void onRender(UIRenderSystem& renderer) = 0;

      // Getters
      glm::vec2 getPosition() const;

      // Setters
      void setPosition(glm::vec2 position);

    protected:
      glm::vec2 m_Position;
  };
}

#endif
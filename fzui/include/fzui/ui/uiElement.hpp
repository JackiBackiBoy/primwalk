#ifndef FZ_UI_ELEMENT_HEADER
#define FZ_UI_ELEMENT_HEADER

// FZUI
#include "fzui/core.hpp"
#include "fzui/rendering/systems/uiRenderSystem.hpp"
#include "fzui/ui/uiEvent.hpp"
#include "fzui/hitbox.hpp"

// std
#include <functional>

// vendor
#include <glm/glm.hpp>

namespace fz {
  // Forward declarations
  class UIContainer;

  class FZ_API UIElement {
    public:
      UIElement(glm::vec2 position, bool draggable = false) : m_Position{position}, m_IsDraggable(draggable) {};
      virtual ~UIElement() {};

      virtual void onRender(UIRenderSystem& renderer) = 0;
      virtual void handleEvent(const UIEvent& event) = 0;
      virtual Hitbox hitboxTest(glm::vec2 position) = 0;

      // Getters
      glm::vec2 getPosition() const;
      glm::vec2 getAbsolutePosition() const;
      virtual Hitbox getHitbox() = 0;
      inline bool isDraggable() const { return m_IsDraggable; }

      // Setters
      void setPosition(glm::vec2 position);
      inline void setOnClick(std::function<void()> onClick) { m_OnClick = onClick; }

    protected:
      glm::vec2 m_Position;
      bool m_IsDraggable;
      std::function<void()> m_OnClick = []() {};
      UIContainer* m_Container = nullptr;

      friend class UIContainer;
  };
}

#endif
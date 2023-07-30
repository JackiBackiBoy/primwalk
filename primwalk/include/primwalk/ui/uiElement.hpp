#ifndef PW_UI_ELEMENT_HEADER
#define PW_UI_ELEMENT_HEADER

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/rendering/systems/uiRenderSystem.hpp"
#include "primwalk/ui/uiEvent.hpp"
#include "primwalk/hitbox.hpp"
#include "primwalk/ui/mouseCursor.hpp"

// std
#include <functional>

// vendor
#include <glm/glm.hpp>

namespace pw {
  // Forward declarations
  class UIContainer;

  class PW_API UIElement {
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
      inline MouseCursor getCursor() const { return m_Cursor; }
      inline bool isDraggable() const { return m_IsDraggable; }
      inline bool retainsFocus() const { return m_RetainsFocus; }

      // Setters
      void setPosition(glm::vec2 position);
      inline void setOnClick(std::function<void()> onClick) { m_OnClick = onClick; }

    protected:
      glm::vec2 m_Position;
      bool m_IsDraggable;
      bool m_RetainsFocus = false;
      std::function<void()> m_OnClick = []() {};
      UIContainer* m_Container = nullptr;
      MouseCursor m_Cursor = MouseCursor::Default;

      friend class UIContainer;
  };
}

#endif
#pragma once

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/hitbox.hpp"
#include "primwalk/rendering/systems/uiRenderSystem.hpp"
#include "primwalk/ui/mouseCursor.hpp"
#include "primwalk/ui/uiEvent.hpp"

// std
#include <functional>

// vendor
#include <glm/glm.hpp>

namespace pw {
  // Forward declarations
  class UIContainer;

  class PW_API UIElement {
    public:
      UIElement(glm::vec2 position = { 0, 0 }, bool draggable = false) : m_Position{position}, m_IsDraggable(draggable) {};
      virtual ~UIElement() {};

      virtual void onRender(UIRenderSystem& renderer) = 0;
      virtual void handleEvent(const UIEvent& event) = 0;
      virtual Hitbox hitboxTest(glm::vec2 position) = 0;

      // Getters
      inline glm::vec2 getPosition() const { return m_Position; }
      glm::vec2 getAbsolutePosition() const;
      inline MouseCursor getCursor() const { return m_Cursor; }
      inline bool isDraggable() const { return m_IsDraggable; }
      inline bool retainsFocus() const { return m_RetainsFocus; }

      // Setters
      inline void setPosition(glm::vec2 position) { m_Position = position; }
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


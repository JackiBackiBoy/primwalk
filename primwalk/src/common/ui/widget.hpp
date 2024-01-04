#pragma once

#include "../rendering/systems/uiRenderSystem.hpp"
#include "uiEvent.hpp"
#include "mouseCursor.hpp"
#include <cstdint>
#include <glm/glm.hpp>

namespace pw {
	typedef int32_t widget_id;

	class Widget {
	public:
		Widget() = default;
		virtual ~Widget() = default;

		virtual void draw(UIRenderSystem& renderer) = 0;
		virtual void processEvent(const UIEvent& event) = 0;
		virtual bool hitTest(glm::vec2 mousePos) = 0;

		
		inline MouseCursor getCursor() const { return m_Cursor; }
		inline glm::vec2 getAbsolutePosition() const {
			glm::vec2 pos = position;

			if (m_Parent != nullptr) {
				pos += m_Parent->getAbsolutePosition();
			}

			return pos;
		}

		inline void setParent(Widget* widget) { m_Parent = widget; }

		glm::vec2 position = { 0, 0 };

	protected:
		// TODO: Optionals might be more suitable here
		widget_id m_ID = -1;
		Widget* m_Parent = nullptr;
		MouseCursor m_Cursor = MouseCursor::Default;

		friend class Editor;
	};

}
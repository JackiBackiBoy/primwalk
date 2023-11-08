#pragma once

// primwalk
#include "../../core.hpp"

// vendor
#include <glm/glm.hpp>

namespace pw {
	class UIElement;

	class PW_API Hitbox {
	public:
		Hitbox(glm::vec2 tl, glm::vec2 bl, glm::vec2 br, glm::vec2 tr, UIElement* element) :
			m_TopLeft(tl), m_BottomLeft(bl), m_BottomRight(br), m_TopRight(tr), m_Element(element) {};
		Hitbox(glm::vec2 tl, int width, int height, UIElement* element) : m_Element(element) {
			m_TopLeft = tl;
			m_BottomLeft = { tl.x, tl.y + height };
			m_BottomRight = { tl.x + width, tl.y + height };
			m_TopRight = { tl.x + width, tl.y };
		};
		~Hitbox() = default;

		inline bool contains(glm::vec2 pos) const {
			return pos.x >= m_TopLeft.x && pos.x < m_TopRight.x &&
			pos.y >= m_TopLeft.y && pos.y < m_BottomLeft.y;
		}

		// Getters
		inline UIElement* getTarget() const { return m_Element; }

	private:
		glm::vec2 m_TopLeft;
		glm::vec2 m_BottomLeft;
		glm::vec2 m_BottomRight;
		glm::vec2 m_TopRight;
		UIElement* m_Element = nullptr;
	};
}


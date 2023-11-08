#include "uiPanel.hpp"

namespace pw {

	void UIPanel::onRender(UIRenderSystem& renderer) {
		// Panel background
		renderer.drawRect(
			getAbsolutePosition(),
			m_Width, m_Height,
			m_BackgroundColor,
			m_BorderRadius);

		// Panel title
		if (!m_Title.empty()) {
			renderer.drawText(
			getAbsolutePosition() + (float)m_BorderRadius * glm::normalize(glm::vec2(1, 1)),
			m_Title, m_FontSize, Color::White, m_Font);
		}
	}

	void UIPanel::handleEvent(const UIEvent& event) {
		return;
	}

	Hitbox UIPanel::hitboxTest(glm::vec2 position) {
		Hitbox hitbox(getAbsolutePosition(), m_Width, m_Height, nullptr);

		if (hitbox.contains(position)) {
			return Hitbox(getAbsolutePosition(), m_Width, m_Height, this);
		}

		return hitbox;
	}

}
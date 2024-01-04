#include "buttonWidget.hpp"
#include "../math/hitbox.hpp"

namespace pw {

	void ButtonWidget::draw(UIRenderSystem& renderer) {
		glm::vec2 absolutePos = getAbsolutePosition();

		renderer.drawRect(absolutePos, width, height, m_BackgroundDisplayColor, borderRadius);

		glm::vec2 centerPos = absolutePos + glm::vec2(width, height) * 0.5f;
		renderer.drawTextCentered(centerPos, text, m_TextDisplayColor, fontSize);
	}

	void ButtonWidget::processEvent(const UIEvent& event) {
		switch (event.getType()) {
		case UIEventType::MouseEnter:
			{
				m_BackgroundDisplayColor = hoverColor;
				m_IsHovered = true;
			}
			break;
		case UIEventType::MouseExit:
			{
				m_BackgroundDisplayColor = backgroundColor;
				m_IsHovered = false;
			}
			break;
		case UIEventType::MouseDown:
			{
				m_IsPressed = true;
				MouseEventData& mouse = event.getMouseData();

				m_BackgroundDisplayColor = clickColor;
			}
			break;
		case UIEventType::MouseUp:
			{
				if (m_IsPressed) {
					m_IsPressed = false;

					// Only call onClick function if left mouse is the cause button
					if (event.getMouseData().causeButtons.leftButton) {
						m_OnClick();
					}

					m_BackgroundDisplayColor = hoverColor;
				}
			}
			break;
		}
	}

	bool ButtonWidget::hitTest(glm::vec2 mousePos) {
		Hitbox hitbox(getAbsolutePosition(), width, height);

		return hitbox.contains(mousePos);
	}

}
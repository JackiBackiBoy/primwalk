#include "checkboxWidget.hpp"
#include "../managers/resourceManager.hpp"
#include <string>

namespace pw {

	CheckboxWidget::CheckboxWidget() {
		m_CheckboxIcon = ResourceManager::Get().loadTexture("assets/icons/check.png");
	}

	void CheckboxWidget::draw(UIRenderSystem& renderer) {
		glm::vec2 pos = getAbsolutePosition();

		renderer.drawRect(pos, size, size, m_BackgroundDisplayColor, borderRadius);

		if (m_Checked) {
			renderer.drawRect(pos, size, size, Color::White, 0, m_CheckboxIcon);
		}

		renderer.drawText({ pos.x + size * 2, pos.y + size / 2 - 6 }, text, 12, Color::White);
	}

	void CheckboxWidget::processEvent(const UIEvent& event) {
		switch (event.getType()) {
		case UIEventType::MouseDown:
			{

			}
			break;
		case UIEventType::MouseUp:
			{
				if (event.getMouseData().causeButtons.leftButton) {
					if (m_Checked) {
						m_BackgroundDisplayColor = backgroundUncheckColor;
					}
					else {
						m_BackgroundDisplayColor = backgroundCheckColor;
					}

					m_Checked = !m_Checked;
				}
			}
			break;
		}
	}

	bool CheckboxWidget::hitTest(glm::vec2 mousePos) {
		Hitbox hitbox(getAbsolutePosition(), size, size);

		return hitbox.contains(mousePos);
	}

}
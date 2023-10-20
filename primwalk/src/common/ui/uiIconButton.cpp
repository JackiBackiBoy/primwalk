#include "primwalk/ui/uiIconButton.hpp"
#include "primwalk/ui/gui.hpp"

namespace pw {

	UIIconButton::UIIconButton() : UIElement() {
		pw::gui::GUIConfig config = pw::gui::GUI::getDefaultConfig();
		m_BackgroundColor = { 255, 255, 255, 0 };
		m_BackgroundHoverColor = config.hoverColor;
		m_BackgroundClickColor = config.clickColor;
		m_BackgroundDisplayColor = m_BackgroundColor;

		m_Cursor = MouseCursor::Hand;
	}

	void UIIconButton::onRender(UIRenderSystem& renderer) {
		renderer.drawRect(getAbsolutePosition(), m_Width, m_Height, m_BackgroundDisplayColor);

		float aspect = (float)m_Icon->getWidth() / m_Icon->getHeight();
		int iconWidth = m_Width - m_Padding * 2;
		int iconHeight = m_Height - m_Padding * 2;

		iconWidth = (aspect > 1.0f) ? iconWidth : aspect * iconHeight;
		iconHeight = (aspect > 1.0f) ? (1.0f / aspect) * iconWidth : iconHeight;

		renderer.drawRect(getAbsolutePosition() + glm::vec2(m_Width / 2 - iconWidth / 2, m_Height / 2 - iconHeight / 2),
			iconWidth, iconHeight, m_IconColor, 0, m_Icon, m_ScissorPos, m_ScissorWidth, m_ScissorHeight);
	}

	void UIIconButton::handleEvent(const UIEvent& event) {
		switch (event.getType()) {
		case UIEventType::MouseEnter:
			{
				m_Hovered = true;
				m_BackgroundDisplayColor = m_BackgroundHoverColor;
			}
			break;
		case UIEventType::MouseExit:
			{
				m_Hovered = false;
				m_Pressed = false;
				m_BackgroundDisplayColor = m_BackgroundColor;
			}
			break;
		case UIEventType::MouseDown:
			{
				m_Pressed = true;
				m_BackgroundDisplayColor = m_BackgroundClickColor;
			}
			break;
		case UIEventType::MouseUp:
			{
				if (m_Pressed) { // button has been clicked
					m_OnClick();
					m_Pressed = false;
					m_BackgroundDisplayColor = m_BackgroundHoverColor;
				}
			}
			break;
		default:
			break;
		}
	}

	Hitbox UIIconButton::hitboxTest(glm::vec2 position) {
		if (Hitbox(getAbsolutePosition(), m_Width, m_Height, nullptr).contains(position)) {
			return Hitbox(getAbsolutePosition(), m_Width, m_Height, this);
		}

		return Hitbox(getAbsolutePosition(), m_Width, m_Height, nullptr);
	}
}
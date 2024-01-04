#include "windowWidget.hpp"
#include <iostream>

namespace pw {

	void WindowWidget::draw(UIRenderSystem& renderer) {
		renderer.drawRect(getAbsolutePosition(), m_Width, m_Height, backgroundColor);
	}

	void WindowWidget::processEvent(const UIEvent& event) {
		switch (event.getType()) {
		case UIEventType::MouseDown:
			{
				if (event.getMouseData().causeButtons.leftButton) {
					if (!m_MouseDownTopOffset.has_value()) {
						m_MouseDownTopOffset = m_Height - (event.getMouseData().position.y - getAbsolutePosition().y);
					}

					if (!m_MouseDownRightOffset.has_value()) {
						m_MouseDownRightOffset = m_Width - (event.getMouseData().position - getAbsolutePosition()).x;
					}

					if (!m_MouseDownOffset.has_value()) {
						glm::vec2 relativeMousePos = (m_Parent != nullptr ? event.getMouseData().position - getAbsolutePosition() : event.getMouseData().position);
						m_MouseDownOffset = position - relativeMousePos;
					}
				}
			}
			break;
		case UIEventType::MouseUp:
		case UIEventType::MouseExit:
			{
				m_MouseDownTopOffset.reset();
				m_MouseDownRightOffset.reset();
				m_MouseDownOffset.reset();
				m_BorderFlags = 0;
			}
			break;
		case UIEventType::MouseDrag:
			{
				if (!event.getMouseData().causeButtons.leftButton) {
					return;
				}

				glm::vec2 mousePos = event.getMouseData().position;
				glm::vec2 relativeMousePos = (m_Parent != nullptr ? mousePos - getAbsolutePosition() : mousePos);

				float top = position.y;
				float left = position.x;
				float right = position.x + m_Width;
				float bottom = position.y + m_Height;

				switch (m_BorderFlags) {
				case 0b00000000:
					{
						position = relativeMousePos + m_MouseDownOffset.value();
					}
					break;
				case 0b01000000: // left
					{
						position.x = relativeMousePos.x + m_MouseDownOffset.value().x;

						if (right - position.x <= minWidth) {
							position.x = right - minWidth;
						}

						setWidth(right - position.x);
					}
					break;
				case 0b00100000: // right
					{
						setWidth((mousePos - getAbsolutePosition()).x + m_MouseDownRightOffset.value());
					}
					break;
				case 0b10000000: // top
					{
						position.y = relativeMousePos.y + m_MouseDownOffset.value().y;

						if (bottom - position.y <= minHeight) {
							position.y = bottom - minHeight;
						}

						setHeight(bottom - position.y);
					}
					break;
				case 0b00010000: // bottom
					{
						setHeight((mousePos - getAbsolutePosition()).y + m_MouseDownTopOffset.value());
					}
					break;
				case 0b11000000: // top-left
					{
						position.x = relativeMousePos.x + m_MouseDownOffset.value().x;
						position.y = relativeMousePos.y + m_MouseDownOffset.value().y;

						if (right - position.x <= minWidth) {
							position.x = right - minWidth;
						}

						if (bottom - position.y <= minHeight) {
							position.y = bottom - minHeight;
						}

						setWidth(right - position.x);
						setHeight(bottom - position.y);
					}
					break;
				case 0b00110000: // bottom-right
					{
						setWidth((mousePos - getAbsolutePosition()).x + m_MouseDownRightOffset.value());
						setHeight((mousePos - getAbsolutePosition()).y + m_MouseDownTopOffset.value());
					}
					break;
				case 0b10100000: // top-right
					{
						position.y = relativeMousePos.y + m_MouseDownOffset.value().y;

						if (bottom - position.y <= minHeight) {
							position.y = bottom - minHeight;
						}

						setWidth((mousePos - getAbsolutePosition()).x + m_MouseDownRightOffset.value());
						setHeight(bottom - position.y);
					}
					break;
				case 0b01010000: // bottom-left
					{
						position.x = relativeMousePos.x + m_MouseDownOffset.value().x;

						if (right - position.x <= minWidth) {
							position.x = right - minWidth;
						}

						setWidth(right - position.x);
						setHeight((mousePos - getAbsolutePosition()).y + m_MouseDownTopOffset.value());
					}
					break;
				}
			}
			break;
		case UIEventType::MouseMove:
			{
				glm::vec2 mousePos = (m_Parent != nullptr ? event.getMouseData().position - getAbsolutePosition() : event.getMouseData().position);

				m_BorderFlags = uint8_t(mousePos.y >= position.y && mousePos.y <= position.y + borderPadding) << 7 | // top
								uint8_t(mousePos.x >= position.x && mousePos.x <= position.x + borderPadding) << 6 | // left
								uint8_t(mousePos.x >= position.x + m_Width - borderPadding && mousePos.x <= position.x + m_Width) << 5 | // right
								uint8_t(mousePos.y >= position.y + m_Height - borderPadding && mousePos.y <= position.y + m_Height) << 4; // bottom

				switch (m_BorderFlags) {
				case 0b11000000: // top-left
				case 0b00110000: // bottom-right
					m_Cursor = MouseCursor::NWSEDiagonalResize;
					break;
				case 0b10100000: // top-right
				case 0b01010000: // bottom-left
					m_Cursor = MouseCursor::NESWDiagonalResize;
					break;
				case 0b00000000: // none
					m_Cursor = MouseCursor::Default;
					break;
				case 0b01000000: // left 
				case 0b00100000: // right
					m_Cursor = MouseCursor::HorizontalResize;
					break;
				case 0b10000000: // top
				case 0b00010000: // bottom
					m_Cursor = MouseCursor::VerticalResize;
					break;
				}
			}
			break;
		}
	}

	bool WindowWidget::hitTest(glm::vec2 mousePos) {
		return Hitbox(getAbsolutePosition(), m_Width, m_Height).contains(mousePos);
	}

}

#include "sliderWidget.hpp"
#include "../managers/resourceManager.hpp"
#include "../math/pwmath.hpp"

#include <algorithm>
#include <sstream>

namespace pw {
	SliderWidget::SliderWidget() {
		m_Font = ResourceManager::Get().findFont("Motiva Sans", FontWeight::Regular);
	}

	void SliderWidget::draw(UIRenderSystem& renderer) {
		float newMaxVal = maxVal - minVal;
		float normCurrentVal = value - minVal;
		float sliderWidth = (normCurrentVal / newMaxVal) * m_Width;

		renderer.drawRect(getAbsolutePosition(), m_Width, m_TrackHeight, m_DisplayTrackColor, 4);
		renderer.drawRect(getAbsolutePosition(), (int)sliderWidth, m_TrackHeight, m_DisplaySliderColor, 4);

		std::ostringstream valueString;
		valueString.precision(precision);
		valueString << std::fixed << value;
		renderer.drawText(getAbsolutePosition() + glm::vec2(m_Width + 20, (m_TrackHeight / 2 - 6)), valueString.str(), 12, Color::White);

		if (m_Hovered) {
			renderer.drawRect(getAbsolutePosition() +
				glm::vec2((normCurrentVal / newMaxVal) * m_Width - m_SliderRadius, m_TrackHeight / 2 - m_SliderRadius), m_SliderRadius * 2, m_SliderRadius * 2, m_SliderColor, m_SliderRadius);
		}
	}

	void SliderWidget::processEvent(const UIEvent& event) {
		switch (event.getType()) {
		case UIEventType::MouseDrag:
			{
				if (m_Pressed) {
					float lastVal = value;

					float mouseX = event.getMouseData().position.x;
					float val = Math::lerp(minVal, maxVal, (event.getMouseData().position.x - getAbsolutePosition().x) / (float)m_Width);
					value = std::clamp(val, minVal, maxVal);

					if (value != lastVal) {
						m_OnChange();
					}
				}
			}
			break;
		case UIEventType::MouseEnter:
			{
				m_Hovered = true;
				m_DisplaySliderColor = m_SliderHoverColor;
			}
			break;
		case UIEventType::MouseExit:
			{
				m_Hovered = false;
				m_Pressed = false;
				m_DisplaySliderColor = m_SliderColor;
			}
			break;
		case UIEventType::MouseDown:
			{
				float lastVal = value;

				m_Pressed = true;
				float mouseX = event.getMouseData().position.x;
				float val = Math::lerp(minVal, maxVal, (event.getMouseData().position.x - getAbsolutePosition().x) / (float)m_Width);
				value = std::clamp(val, minVal, maxVal);

				if (value != lastVal) {
					m_OnChange();
				}
			}
			break;
		case UIEventType::MouseUp:
			{
				if (m_Pressed) {
					m_Pressed = false;
				}
			}
			break;
		//case UIEventType::MouseWheel:
		//{
		//	float percentage = event.getMouseData().wheelDelta.y / 100.0f;
		//	float normRange = m_MaxVal - m_MinVal;
		//	float normCurrentVal = m_CurrentVal - m_MinVal;
		//	float roundPercentage = int((normCurrentVal / normRange) * 10.0f) / 10.0f;

		//	m_CurrentVal = std::clamp(Math::lerp(m_MinVal, m_MaxVal, (roundPercentage + percentage)), m_MinVal, m_MaxVal);
		//}
		break;
		}
	}

	bool SliderWidget::hitTest(glm::vec2 mousePos) {
		return Hitbox(getAbsolutePosition() + glm::vec2(0, m_TrackHeight / 2 - m_SliderRadius), m_Width, m_SliderRadius * 2).contains(mousePos);
	}
}
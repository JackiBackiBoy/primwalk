#include "primwalk/ui/uiSlider.hpp"
#include "primwalk/math/pwmath.hpp"

// std
#include <algorithm>
#include <iostream>

namespace pw {

	void UISlider::onRender(UIRenderSystem& renderer) {
		float newMaxVal = m_MaxVal - m_MinVal;
		float normCurrentVal = m_CurrentVal - m_MinVal;
		float sliderWidth = (normCurrentVal / newMaxVal) * m_Width;

		renderer.drawRect(getAbsolutePosition(), m_Width, m_TrackHeight, m_DisplayTrackColor, 4);
		renderer.drawRect(getAbsolutePosition(), (int)sliderWidth, m_TrackHeight, m_DisplaySliderColor, 4);
		renderer.drawText(getAbsolutePosition() - glm::vec2(100, 0), std::to_string(m_CurrentVal), 15, Color::White);

		if (m_Hovered) {
			renderer.drawRect(getAbsolutePosition() +
				glm::vec2((normCurrentVal / newMaxVal) * m_Width - m_SliderRadius, m_TrackHeight / 2 - m_SliderRadius),
				m_SliderRadius * 2, m_SliderRadius * 2, m_SliderColor, m_SliderRadius + 1);
		}
	}

	void UISlider::handleEvent(const UIEvent& event) {
		switch (event.getType()) {
		case UIEventType::MouseDrag:
			{
				if (m_Pressed) {
					float mouseX = event.getMouseData().position.x;
					float val = Math::lerp(m_MinVal, m_MaxVal, (event.getMouseData().position.x - getAbsolutePosition().x) / (float)m_Width);
					m_CurrentVal = std::clamp(val, m_MinVal, m_MaxVal);
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
				m_Pressed = true;
				float mouseX = event.getMouseData().position.x;
				float val = Math::lerp(m_MinVal, m_MaxVal, (event.getMouseData().position.x - getAbsolutePosition().x) / (float)m_Width);
				m_CurrentVal = std::clamp(val, m_MinVal, m_MaxVal);
			}
			break;
		case UIEventType::MouseUp:
			{
				if (m_Pressed) {
					m_Pressed = false;
				}
			}
			break;
		case UIEventType::MouseWheel:
			{
				float percentage = event.getMouseData().wheelDelta.y / 100.0f;
				float normRange = m_MaxVal - m_MinVal;
				float normCurrentVal = m_CurrentVal - m_MinVal;
				float roundPercentage = int((normCurrentVal / normRange) * 10.0f) / 10.0f;

				m_CurrentVal = std::clamp(Math::lerp(m_MinVal, m_MaxVal, (roundPercentage + percentage)), m_MinVal, m_MaxVal);
			}
			break;
		default:
			break;
		}
	}

	Hitbox UISlider::hitboxTest(glm::vec2 position) {
		if (Hitbox(getAbsolutePosition() + glm::vec2(0, m_TrackHeight / 2 - m_SliderRadius), m_Width, m_SliderRadius * 2, nullptr).contains(position)) {
			return Hitbox(getAbsolutePosition() + glm::vec2(0, m_TrackHeight / 2 - m_SliderRadius), m_Width, m_SliderRadius * 2, this);
		}

		return Hitbox(getAbsolutePosition() + glm::vec2(0, m_TrackHeight / 2 - m_SliderRadius), m_Width, m_SliderRadius * 2, nullptr);
	}

}

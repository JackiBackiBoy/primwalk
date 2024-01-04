#pragma once

#include "widget.hpp"
#include "../data/font.hpp"
#include "../color.hpp"

#include <functional>

namespace pw {
	class SliderWidget : public Widget {
	public:
		SliderWidget();
		~SliderWidget() = default;

		void draw(UIRenderSystem& renderer) override;
		void processEvent(const UIEvent& event) override;
		bool hitTest(glm::vec2 mousePos) override;

		inline float getValue() const { return value; }

		inline void setOnChange(std::function<void()> onChange) { m_OnChange = onChange; }

		unsigned int precision = 0;
		float value = 50;
		float minVal = 0;
		float maxVal = 100;

	private:
		int m_Width = 100;
		int m_TrackHeight = 4;
		int m_SliderRadius = 6;

		bool m_Hovered = false;
		bool m_Pressed = false;

		Color m_TrackColor = { 128, 128, 128 };
		Color m_TrackHoverColor = m_TrackColor;
		Color m_SliderColor = { 255, 255, 255 };
		Color m_SliderHoverColor = { 118, 37, 168 };

		Color m_DisplayTrackColor = m_TrackColor;
		Color m_DisplaySliderColor = m_SliderColor;

		std::shared_ptr<Font> m_Font = nullptr;
		std::function<void()> m_OnChange = []() {};
	};
}
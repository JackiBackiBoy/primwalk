#pragma once

#include "widget.hpp"
#include <functional>
#include <string>

namespace pw {
	class ButtonWidget : public Widget {
	public:
		ButtonWidget() { m_Cursor = MouseCursor::Hand; }
		~ButtonWidget() = default;
		
		void draw(UIRenderSystem& renderer) override;
		void processEvent(const UIEvent& event) override;
		bool hitTest(glm::vec2 mousePos) override;

		std::string text = "Button";
		int width = 100;
		int height = 50;
		int borderRadius = 0;
		double fontSize = 14;
		Color backgroundColor = { 52, 52, 52 };
		Color textColor = Color::White;
		Color hoverColor = { 70, 70, 70 };
		Color clickColor = { 90, 90, 90 };

		inline void setOnClick(std::function<void()> onClick) { m_OnClick = onClick; }

	private:
		Color m_BackgroundDisplayColor = backgroundColor;
		Color m_TextDisplayColor = textColor;
		bool m_IsHovered = false;
		bool m_IsPressed = false;
		std::function<void()> m_OnClick = []() {};
	};
}
#pragma once

// primwalk
#include "../../core.hpp"
#include "../color.hpp"
#include "../data/font.hpp"
#include "uiElement.hpp"

// std
#include <memory>
#include <string>

namespace pw {
	class PW_API UILabel : public UIElement {
	public:
		UILabel();
		virtual ~UILabel() {};

		virtual void onRender(UIRenderSystem& renderer) override;
		virtual void handleEvent(const UIEvent& event) override;
		virtual Hitbox hitboxTest(glm::vec2 position) override;

		// Setters
		inline void setText(const std::string& text) { m_Text = text; }
		void setBackgroundColor(Color color);
		inline void setTextColor(Color color) { m_TextColor = color; }
		inline void setFont(std::shared_ptr<Font> font) { m_Font = font; }
		inline void setFontSize(double size) { m_FontSize = size; }

	private:
		std::string m_Text = "";
		std::shared_ptr<Font> m_Font = nullptr;
		double m_FontSize = 0;

		// Colors
		Color m_BackgroundColor = { 255, 255, 255 };
		Color m_TextColor = { 255, 255, 255 };
	};
}

#pragma once

// primwalk
#include "../../core.hpp"
#include "../color.hpp"
#include "../managers/resourceManager.hpp"
#include "uiElement.hpp"

// std
#include <memory>
#include <string>
#include <mutex>

namespace pw {
	class PW_API UIButton : public UIElement {
	public:
		UIButton();
		~UIButton() = default;

		virtual void onRender(UIRenderSystem& renderer) override;
		virtual void handleEvent(const UIEvent& event) override;
		virtual Hitbox hitboxTest(glm::vec2 position) override;

		// Setters
		inline void setText(const std::string& text) { m_Text = text; }
		void setBackgroundColor(Color color);
		inline void setBackgroundHoverColor(Color color) { m_BackgroundHoverColor = color; }
		inline void setBackgroundClickColor(Color color) { m_BackgroundClickColor = color; }
		inline void setTextColor(Color color) { m_TextColor = color; }
		inline void setTextHoverColor(Color color) { m_TextHoverColor = color; }
		inline void setBorderRadius(int radius) { m_BorderRadius = radius; }
		inline void setIcon(std::shared_ptr<Texture2D> texture) { m_Icon = texture; }

	private:
		std::string m_Text;
		std::string m_FontName;
		int m_Width = 100;
		int m_Height = 50;
		int m_BorderRadius = 0;
		double m_FontSize;
		bool m_Hovered = false;
		bool m_Pressed = false;
		std::mutex m_PropertyMutex;
		std::shared_ptr<Texture2D> m_Icon = nullptr;
		std::shared_ptr<Font> m_Font = nullptr;

		// Colors
		Color m_BackgroundColor;
		Color m_BackgroundHoverColor;
		Color m_BackgroundClickColor;
		Color m_TextColor;
		Color m_TextHoverColor;
		Color m_DisplayColor;
	};
}


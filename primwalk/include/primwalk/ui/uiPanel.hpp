#pragma once

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/color.hpp"
#include "primwalk/data/font.hpp"
#include "primwalk/ui/uiElement.hpp"

// std
#include <string>

namespace pw {
	class PW_API UIPanel : public UIElement {
	public:
		UIPanel() : UIElement() {};
		~UIPanel() {};

		void onRender(UIRenderSystem& renderer) override;
		void handleEvent(const UIEvent& event) override;
		Hitbox hitboxTest(glm::vec2 position) override;

		inline int getWidth() const { return m_Width; }
		inline int getHeight() const { return m_Height; }
		inline int getBorderRadius() const { return m_BorderRadius; }
		inline std::string getTitle() const { return m_Title; }
		inline Color getBackgroundColor() const { return m_BackgroundColor; }

		inline void setWidth(int width) { m_Width = width; }
		inline void setHeight(int height) { m_Height = height; }
		inline void setBorderRadius(int radius) { m_BorderRadius = radius; }
		inline void setFontSize(double size) { m_FontSize = size; }
		inline void setTitle(const std::string& title) { m_Title = title; }
		inline void setBackgroundColor(Color color) { m_BackgroundColor = color; }

	private:
		int m_Width = 100;
		int m_Height = 100;
		int m_BorderRadius = 0;
		double m_FontSize = 12;
		std::string m_Title = "Panel";
		std::shared_ptr<Font> m_Font = nullptr;

		Color m_BackgroundColor = Color::White;
	};
}
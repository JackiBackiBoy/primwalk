#pragma once

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/ui/uiElement.hpp"
#include "primwalk/data/font.hpp"

// std
#include <memory>
#include <string>
#include <vector>

namespace pw {
	struct PW_API ListItem {
		std::string text = "";
		std::shared_ptr<Texture2D> icon = nullptr;
		glm::vec2 iconScissorPos = { 0, 0 };
		int iconScissorWidth = 1;
		int iconScissorHeight = 1;
		std::function<void()> onClick = []() {};
	};

	class PW_API UIListBox : public UIElement {
	public:
		UIListBox();
		~UIListBox() = default;

		void onRender(UIRenderSystem& renderer) override;
		void handleEvent(const UIEvent& event) override;
		Hitbox hitboxTest(glm::vec2 position) override;

		void addItem(ListItem item);
		inline void removeAllItems() { m_Items.clear(); }

		inline int getWidth() const { return m_Width; }
		inline int getHeight() const { return m_Height; }
		inline int getFontSize() const { return m_FontSize; }
		inline Color getBackgroundColor() const { return m_BackgroundColor; }

		inline void setWidth(int width) { m_Width = width; }
		inline void setHeight(int height) { m_Height = height; }
		inline void setFontSize(double size) { m_FontSize = size; }
		inline void setBackgroundColor(Color color) { m_BackgroundColor = color; }

	private:
		int m_Width = 100;
		int m_Height = 100;
		int m_ItemHeight = 20;
		double m_FontSize = 12;

		Color m_BackgroundColor = { 100, 100, 100 };
		Color m_ItemHoverColor;


		std::shared_ptr<Font> m_Font = nullptr;
		ListItem* m_CurrentItem = nullptr;

		std::vector<ListItem> m_Items{};
	};
}
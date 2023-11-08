#include "uiListBox.hpp"
#include "GUI.hpp"
#include "../managers/resourceManager.hpp"


namespace pw {

	UIListBox::UIListBox() : UIElement() {
		auto config = pw::gui::GUI::getDefaultConfig();

		m_ItemHoverColor = config.hoverColor;
	}

	void UIListBox::onRender(UIRenderSystem& renderer) {
		// Background
		renderer.drawRect(
			getAbsolutePosition(),
			m_Width,
			m_Height,
			m_BackgroundColor);

		glm::vec2 itemPos = getAbsolutePosition() + glm::vec2(10, 0);

		for (const auto& i : m_Items) {
			if (m_CurrentItem == &i) {
			renderer.drawRect(itemPos, m_Width, m_ItemHeight, m_ItemHoverColor);
			}

			if (i.icon != nullptr) {
				renderer.drawRect(
					itemPos, m_ItemHeight, m_ItemHeight,
					Color::White, 0,
					i.icon, i.iconScissorPos, i.iconScissorWidth, i.iconScissorHeight);
			}

			if (!i.text.empty()) {
				if (m_Font == nullptr) {
					m_Font = ResourceManager::Get().findFont("Motiva Sans", FontWeight::Regular);
				}

				glm::vec2 textPos = itemPos + glm::vec2(m_ItemHeight + 4,
					m_ItemHeight / 2 - (m_Font->getMaxHeight() * (m_FontSize / m_Font->getFontSize())) / 2);
				renderer.drawText(textPos, i.text, m_FontSize, Color::White, m_Font);
			}

			itemPos.y += m_ItemHeight;
		}
	}

	void UIListBox::handleEvent(const UIEvent& event) {
		switch (event.getType()) {
		case UIEventType::MouseMove:
			{
				glm::vec2 mousePos = event.getMouseData().position;
				glm::vec2 itemPos = getAbsolutePosition() + glm::vec2(10, 0);
				bool hitItem = false;

				for (auto& i : m_Items) {
					if (Hitbox(itemPos, m_Width, m_ItemHeight, nullptr).contains(mousePos)) {
						m_CurrentItem = &i;
						hitItem = true;
						break;
					}

					itemPos.y += m_ItemHeight;
				}

				if (!hitItem) {
					m_CurrentItem = nullptr;
				}
			}
			break;
		case UIEventType::MouseExit:
			{
				m_CurrentItem = nullptr;
			}
			break;
		case UIEventType::MouseDown:
			{
				if (m_CurrentItem != nullptr) {
					m_CurrentItem->onClick();
				}
			}
			break;
		default:
			break;
		}
	}

	Hitbox UIListBox::hitboxTest(glm::vec2 position) {
		Hitbox hitbox(getAbsolutePosition(), m_Width, m_Height, nullptr);

		if (hitbox.contains(position)) {
			return Hitbox(getAbsolutePosition(), m_Width, m_Height, this);
		}

		return hitbox;
	}

	void UIListBox::addItem(ListItem item) {
		m_Items.push_back(item);
	}

}
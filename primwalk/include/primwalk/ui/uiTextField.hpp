#pragma once

// primwalk
#include "primwalk/color.hpp"
#include "primwalk/core.hpp"
#include "primwalk/ui/uiElement.hpp"
#include "primwalk/managers/resourceManager.hpp"

// std
#include <chrono>
#include <memory>
#include <string>

namespace pw {
	class PW_API UITextField : public UIElement {
	public:
		UITextField(glm::vec2 position, int width, int height, const std::string& defaultText = "", std::shared_ptr<Font> font = nullptr) :
			UIElement(position, true), m_Width(width), m_Height(height), m_DefaultText(defaultText), m_Font(font)
		{
			m_Cursor = MouseCursor::IBeam;
			m_RetainsFocus = true;

			if (m_Font == nullptr) {
				m_Font = ResourceManager::Get().findFont("Catamaran", FontWeight::Bold);
			}
		};

		virtual ~UITextField() = default;

		void onRender(UIRenderSystem& renderer) override;
		void handleEvent(const UIEvent& event) override;
		Hitbox hitboxTest(glm::vec2 position) override;

	private:
		int m_Width;
		int m_Height;
		std::string m_DefaultText;
		std::shared_ptr<Font> m_Font;
		bool m_IsFocused = false;
		bool m_IsHighlighting = false;
		size_t m_CaretIndex = 0;
		int m_SelectionStart = 0;
		int m_SelectionEnd = 0;

		std::chrono::time_point<std::chrono::high_resolution_clock> m_LastRenderTime{};
		float m_CaretTimer = 0.0f;
		float m_CaretBlinkDelay = 0.5f;
		std::string m_Text = "";

		Color m_BorderColor = { 128, 128, 128 };
		Color m_BackgroundColor = { 40, 40, 40 };
		Color m_TextColor = { 0, 0, 0 };

		Color m_BorderHoverColor = { 0, 156, 255 };
		Color m_BackgroundHoverColor = m_BackgroundColor;
		Color m_TextHoverColor = m_TextColor;

		Color m_DisplayBorderColor = m_BorderColor;
		Color m_DisplayBackgroundColor = m_BackgroundColor;
		Color m_DisplayTextColor = m_TextColor;
	};
}


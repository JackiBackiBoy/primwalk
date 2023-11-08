#pragma once

// primwalk
#include "../../core.hpp"
#include "../color.hpp"
#include "../rendering/systems/uiRenderSystem.hpp"
#include "uiElement.hpp"
#include "uiEvent.hpp"

namespace pw::gui {
	UIElement* hitTest(glm::vec2 cursorPos);
	bool processEvent(const UIEvent& event);

	struct PW_API GUIConfig {
		double smallFontSize = 12;
		double mediumFontSize = 24;
		double largeFontSize = 32;

		Color textColor = Color::White;
		Color primaryColor = { 60, 60, 60 };
		Color secondaryColor = { 100, 100, 100 };
		Color accentColor = { 104, 37, 196 };
		Color hoverColor = { 120, 120, 120 };
		Color clickColor = { 130, 130, 130 };
	};

	class PW_API GUI {
	public:
		GUI() {};
		~GUI() = default;

		void addWidget(UIElement* widget);
		void onUpdate();
		void onRender(UIRenderSystem& renderer);

		inline static GUIConfig getDefaultConfig() { return defaultConfig; }
		inline static void setDefaultConfig(const GUIConfig& config) { defaultConfig = config; }


	private:
		static GUIConfig defaultConfig;
	};
}
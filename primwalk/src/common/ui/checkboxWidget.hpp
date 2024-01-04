#pragma once

#include "widget.hpp"
#include "../rendering/texture2D.hpp"
#include "../color.hpp"

#include <memory>

namespace pw {
	class CheckboxWidget : public Widget {
	public:
		CheckboxWidget();
		~CheckboxWidget() = default;

		void draw(UIRenderSystem& renderer) override;
		void processEvent(const UIEvent& event) override;
		bool hitTest(glm::vec2 mousePos) override;

		int size = 16;
		int borderRadius = 4;
		Color backgroundCheckColor = { 118, 37, 168 };
		Color backgroundUncheckColor = { 80, 80, 80 };
		Color checkColor = { 255, 255, 255 };
		Color borderColor = { 200, 200, 200 };
		std::string text = "Checkbox";

	private:
		bool m_Checked = false;
		Color m_BackgroundDisplayColor = backgroundUncheckColor;
		std::shared_ptr<Texture2D> m_CheckboxIcon;
	};
}
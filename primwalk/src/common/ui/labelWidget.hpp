#pragma once

#include "widget.hpp"
#include "../color.hpp"

namespace pw {
	class LabelWidget : public Widget {
	public:
		LabelWidget() = default;
		~LabelWidget() = default;

		void draw(UIRenderSystem& renderer) override;
		void processEvent(const UIEvent& event) override;
		bool hitTest(glm::vec2 mousePos) override;

		std::string text = "Label";
		double fontSize = 14;
		Color color = Color::White;

	private:


	};
}
#pragma once

#include "widget.hpp"
#include "../color.hpp"

namespace pw {
	class ContainerWidget : public Widget {
	public:
		ContainerWidget() = default;
		~ContainerWidget() = default;

		void draw(UIRenderSystem& renderer) override;
		void processEvent(const UIEvent& event) override;
		bool hitTest(glm::vec2 mousePos) override;

		int width = 100;
		int height = 100;
		int borderRadius = 0;
		Color backgroundColor = { 60, 60, 60 };

	private:
	};
}
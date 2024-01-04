#include "containerWidget.hpp"

namespace pw {

	void ContainerWidget::draw(UIRenderSystem& renderer) {
		renderer.drawRect(position, width, height, backgroundColor, borderRadius);
	}

	void ContainerWidget::processEvent(const UIEvent& event) {
		return;
	}

	bool ContainerWidget::hitTest(glm::vec2 mousePos) {
		return false;
	}

}
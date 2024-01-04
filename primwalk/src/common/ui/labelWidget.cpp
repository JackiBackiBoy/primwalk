#include "labelWidget.hpp"

namespace pw {

	void LabelWidget::draw(UIRenderSystem& renderer) {
		renderer.drawText(getAbsolutePosition(), text, fontSize, color);
	}

	void LabelWidget::processEvent(const UIEvent& event) {
		return;
	}

	bool LabelWidget::hitTest(glm::vec2 mousePos) {
		return false;
	}

}
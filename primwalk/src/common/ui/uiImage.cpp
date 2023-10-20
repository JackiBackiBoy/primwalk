#include "primwalk/ui/uiImage.hpp"

namespace pw {

	UIImage::UIImage() : UIElement() {
		m_Cursor = MouseCursor::Default;
	}

	void UIImage::onRender(UIRenderSystem& renderer) {
		renderer.drawRect(getAbsolutePosition(), m_Width, m_Height, m_Color, 0, m_Image);
	}

	void UIImage::handleEvent(const UIEvent& event) {
		return;
	}

	Hitbox UIImage::hitboxTest(glm::vec2 position) {
		return Hitbox(getAbsolutePosition(), m_Width, m_Height, nullptr);
	}
}
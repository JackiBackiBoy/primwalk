#include "primwalk/ui/uiElement.hpp"
#include "primwalk/ui/uiContainer.hpp"

namespace pw {
	glm::vec2 UIElement::getAbsolutePosition() const {
		if (m_Container != nullptr) {
			return m_Position + m_Container->getAbsolutePosition();
		}

		return m_Position;
	}
}
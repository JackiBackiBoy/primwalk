#include "GUI.hpp"

namespace pw::gui {
	UIElement* m_TargetElement = nullptr;
	UIElement* m_FocusElement = nullptr;
	std::vector<UIElement*> m_Widgets;

	UIElement* hitTest(glm::vec2 mousePos) {
		for (auto w = m_Widgets.rbegin(); w != m_Widgets.rend(); ++w) {
			Hitbox hitbox = (*w)->hitboxTest(mousePos);

			if (hitbox.getTarget() != nullptr) {
				return hitbox.getTarget();
			}
		}

		return nullptr;
	}

	bool processEvent(const UIEvent& event) {
		switch (event.getType()) {
		case UIEventType::FocusLost:
			{
				if (m_FocusElement != nullptr) {
					m_FocusElement->handleEvent({ UIEventType::FocusLost });
				}
			}
			break;
		case UIEventType::KeyboardDown:
		case UIEventType::KeyboardChar:
			{
				if (m_FocusElement != nullptr) {
				m_FocusElement->handleEvent(event);
				}
			}
			break;
		case UIEventType::MouseDown:
		case UIEventType::MouseUp:
		case UIEventType::MouseMove:
		case UIEventType::MouseWheel:
		case UIEventType::MouseDrag:
		case UIEventType::MouseExitWindow:
			{
				// Perform hit tests
				glm::vec2 mousePos = event.getMouseData().position;
				bool elementFound = false;

				for (auto w = m_Widgets.rbegin(); w != m_Widgets.rend(); ++w) {
					Hitbox hitbox = (*w)->hitboxTest(mousePos);

					if (event.getType() == UIEventType::MouseDrag && m_TargetElement != nullptr) {
						if (m_TargetElement->isDraggable()) {
							m_TargetElement->handleEvent(event);
							return true;
						}
					}

					if (hitbox.getTarget() != nullptr) {
						elementFound = true;

						if (event.getType() == UIEventType::MouseDown) {
							if (m_TargetElement != nullptr) {
								if (m_TargetElement->retainsFocus()) {
									if (m_FocusElement != nullptr) {
										m_FocusElement->handleEvent({ UIEventType::FocusLost });
									}

									m_FocusElement = m_TargetElement;
								}

								if (m_FocusElement != nullptr && m_FocusElement != m_TargetElement) {
									m_FocusElement->handleEvent({ UIEventType::FocusLost });
									m_FocusElement = nullptr;
								}

								m_TargetElement->handleEvent(event);
							}
						}

						if (hitbox.getTarget() != m_TargetElement) {
							if (m_TargetElement != nullptr) {
								m_TargetElement->handleEvent({ UIEventType::MouseExit });
							}

							m_TargetElement = hitbox.getTarget();
							UIEvent tempEvent(UIEventType::MouseEnter);
							tempEvent.getMouseData().position = event.getMouseData().position;
							m_TargetElement->handleEvent(tempEvent);
						}
						else if (event.getType() == UIEventType::MouseUp) {
							m_TargetElement->handleEvent({ UIEventType::MouseUp });
						}
						else if (event.getType() == UIEventType::MouseMove) {
							m_TargetElement->handleEvent(event);
						}
						else if (event.getType() == UIEventType::MouseWheel) {
							m_TargetElement->handleEvent(event);
						}

						break;
					}
				}

				// If no elements were hit, set target element as nullptr
				if (!elementFound) {
					if (m_TargetElement != nullptr) {
						m_TargetElement->handleEvent({ UIEventType::MouseExit });
					}

					if (m_FocusElement != nullptr && event.getType() == UIEventType::MouseDown) {
						m_FocusElement->handleEvent({ UIEventType::FocusLost });
						m_FocusElement = nullptr;
					}

					m_TargetElement = nullptr;
				}
				else {
					return true;
				}
			}
			break;
		default:
			break;
		}

		return false;
	}

	void GUI::addWidget(UIElement* widget) {
		m_Widgets.push_back(widget);
	}

	void GUI::onUpdate() {

	}

	void GUI::onRender(UIRenderSystem& renderer) {
		for (const auto& w : m_Widgets) {
			w->onRender(renderer);
		}
	}

	GUIConfig GUI::defaultConfig{};
}
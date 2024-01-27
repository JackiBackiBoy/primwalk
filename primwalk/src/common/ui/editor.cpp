#include "editor.hpp"
#include "buttonWidget.hpp"
#include "checkboxWidget.hpp"
#include "containerWidget.hpp"
#include "labelWidget.hpp"
#include "sliderWidget.hpp"
#include "windowWidget.hpp"
#include <iostream>

namespace pw {

	Editor::Editor() {
		auto properties = addWidget<ContainerWidget>();
		properties->position = { 20, 300 };
		properties->width = 200;
		properties->height = 300;
		properties->borderRadius = 8;

		auto button = addWidget<ButtonWidget>();
		button->text = "Primwalk";
		button->position = { 10, 10 };
		button->borderRadius = 10;
		button->setOnClick([]() { std::cout << "huh\n"; });
		button->setParent(properties);

		auto label = addWidget<LabelWidget>();
		label->position = { 100, 200 };
		label->text = "Testing label!";

		// Scene Explorer Window
		auto sceneExplorer = addWidget<WindowWidget>();
		sceneExplorer->position = { 500, 300 };
		sceneExplorer->setWidth(300);
		sceneExplorer->setHeight(300);

		auto check = addWidget<CheckboxWidget>();
		check->setParent(sceneExplorer);
		check->position = { 10, 10 };
		check->text = "Specular Lighting";

		auto hitboxCheck = addWidget<CheckboxWidget>();
		hitboxCheck->setParent(sceneExplorer);
		hitboxCheck->position = { 10, 35 };
		hitboxCheck->text = "Show Hitboxes";

		auto specularSliderLabel = addWidget<LabelWidget>();
		specularSliderLabel->setParent(sceneExplorer);
		specularSliderLabel->text = "Specular Intensity:";
		specularSliderLabel->position = { 10, 70 };
		specularSliderLabel->fontSize = 13;

		auto specularSlider = addWidget<SliderWidget>();
		specularSlider->setParent(specularSliderLabel);
		specularSlider->position = { 0, 30 };
		specularSlider->minVal = 0;
		specularSlider->maxVal = 255;
		//specularSlider->setOnChange([this, specularSlider]() { m_Target->m_DeferredPass->getOffscreenPass()->setClearColor(
		//	0, { (unsigned char)specularSlider->getValue(), (unsigned char)specularSlider->getValue(), (unsigned char)specularSlider->getValue() }); }
		//);

		auto ambientSliderLabel = addWidget<LabelWidget>();
		ambientSliderLabel->setParent(sceneExplorer);
		ambientSliderLabel->text = "Ambient Lighting:";
		ambientSliderLabel->position = { 10, 150 };
		ambientSliderLabel->fontSize = 13;

		auto ambientIntensityLabel = addWidget<LabelWidget>();
		ambientIntensityLabel->setParent(sceneExplorer);
		ambientIntensityLabel->text = "Intensity:";
		ambientIntensityLabel->position = { 10, 175 };
		ambientIntensityLabel->fontSize = 12;

		auto ambientIntensitySlider = addWidget<SliderWidget>();
		ambientIntensitySlider->setParent(ambientSliderLabel);
		ambientIntensitySlider->position = { 0, 50 };
		ambientIntensitySlider->value = 0.5f;
		ambientIntensitySlider->minVal = 0.0f;
		ambientIntensitySlider->maxVal = 1.0f;
		ambientIntensitySlider->precision = 2;

		// Graphics Settings
		auto graphicsSettingsLabel = addWidget<LabelWidget>();
		graphicsSettingsLabel->setParent(sceneExplorer);
		graphicsSettingsLabel->text = "Graphics Settings";
		graphicsSettingsLabel->fontSize = 13;
		graphicsSettingsLabel->position = { 10, 220 };
	}

	Editor& Editor::getInstance() {
		if (m_Instance == nullptr) {
			m_Instance = new Editor();
		}

		return *m_Instance;
	}

	WidgetEventData Editor::processEvent(const UIEvent& event) {
		switch (event.getType()) {
		case UIEventType::MouseMove:
			{
				glm::vec2 mousePos = event.getMouseData().position;
				Widget* hitWidget = nullptr;

				for (auto w = m_Widgets.rbegin(); w != m_Widgets.rend(); w++) {
					if (w->get()->hitTest(mousePos)) {
						hitWidget = w->get();
						w->get()->processEvent({ UIEventType::MouseEnter });
						break;
					}
				}

				if (m_CurrentWidget != hitWidget) {
					if (m_CurrentWidget != nullptr) {
						m_CurrentWidget->processEvent({ UIEventType::MouseExit });
					}

					m_CurrentWidget = hitWidget;
				}

				if (hitWidget != nullptr) {
					m_CurrentEventData.cursorRequest = hitWidget->getCursor();
				}
				else {
					m_CurrentEventData.cursorRequest = MouseCursor::Default;
				}

				if (m_CurrentWidget != nullptr) {
					m_CurrentWidget->processEvent({ event });
				}
			}
			break;
		case UIEventType::MouseDrag:
			{
				if (m_CurrentWidget != nullptr) {
					m_CurrentWidget->processEvent(event);
				}
			}
			break;
		case UIEventType::MouseDown:
		case UIEventType::MouseUp:
			{
				if (m_CurrentWidget != nullptr) {
					m_CurrentWidget->processEvent(event);
				}
			}
			break;
		}

		return m_CurrentEventData;
	}

	void Editor::draw(UIRenderSystem& renderer) {
		// TODO: We will run into problems with this in terms of rendering order, fix it
		for (const auto& w : m_Widgets) {
			w->draw(renderer);
		}
	}

	void Editor::destroy() {
		delete m_Instance;
	}

}
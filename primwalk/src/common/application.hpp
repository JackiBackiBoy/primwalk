#pragma once

// primwalk
#include "../core.hpp"
#include "../window.hpp"
#include "components/entity.hpp"
#include "data/model.hpp"
#include "managers/componentManager.hpp"
#include "managers/entityManager.hpp"
#include "managers/systemManager.hpp"
#include "rendering/graphicsDevice_Vulkan.hpp"
#include "rendering/renderer.hpp"
#include "rendering/systems/uiRenderSystem.hpp"
#include "rendering/deferredPass.hpp"

#include "systems/renderSystem3d.hpp"
#include "ui/GUI.hpp"
#include "ui/menuWidget.hpp"
#include "ui/uiElement.hpp"
#include "ui/uiEvent.hpp"
#include "ui/uiIconButton.hpp"
#include "ui/uiImage.hpp"
#include "ui/uiLabel.hpp"
#include "ui/uiListBox.hpp"
#include "ui/uiPanel.hpp"
#include "ui/uiSlider.hpp"

// std
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace pw {
	class PW_API Application {
	public:
		Application();
		virtual ~Application();

		/* Called once at application startup */
		virtual void onStart() = 0;

		/* Called once per frame */
		virtual void onUpdate(float dt) = 0;

		/* Called 60 times per second (primarily used for physics) */
		virtual void onFixedUpdate(float dt) = 0;

		void initialize();
		void run();

		Entity* createEntity(const std::string& name);
		Entity* createLightEntity(const std::string& name);

	private:
		void onRender(float dt);
		void updateScene(float dt);

		std::unique_ptr<Window> m_Window;
		std::unique_ptr<GraphicsDevice> m_Device;
		std::unique_ptr<Renderer> m_Renderer;
		std::unique_ptr<UIRenderSystem> m_UIRenderSystem;
		std::unique_ptr<Model> m_Cube;
		std::unique_ptr<DeferredPass> m_DeferredPass;

		// GUI
		// TODO: Move all editor GUI to a more suitable place, i.e. an Editor class
		UILabel testLabel;
		UIImage engineLogo;
		UIIconButton fullscreenButton;
		UIIconButton minimizeButton;
		UIIconButton maximizeButton;
		UIIconButton closeButton;
		UIIconButton selectButton;
		UIIconButton moveButton;
		UIIconButton rotateButton;
		UIIconButton scaleButton;
		MenuWidget menu;
		MenuItem fileMenu;
		MenuItem fileSubNewMenu;
		MenuItem fileSubOpenMenu;
		MenuItem editMenu;
		MenuItem assetsMenu;
		MenuItem toolsMenu;
		MenuItem windowMenu;
		MenuItem helpMenu;
		MenuItem helpView;
		MenuItem helpGettingStarted;
		MenuItem helpTipsAndTricks;
		MenuItem helpKeyboardShortcuts;
		MenuItem helpWhatsNew;
		MenuItem helpCheckForUpdates;
		MenuItem helpReleaseNotes;
		MenuItem helpRoadmap;
		MenuItem helpAbout;
		UIIconButton playButton;
		UIIconButton pauseButton;
		UIPanel sceneExplorer;
		UIListBox sceneEntitiesListBox;
		UIPanel propertiesPanel;
		pw::gui::GUI m_GUI;
		bool m_ScenePaused = false;
		bool m_DebugMode = false;

		// Entities
		std::vector<std::unique_ptr<Entity>> m_Entities{};
		std::shared_ptr<Texture2D> icons = nullptr;

		ComponentManager m_ComponentManager{};
		EntityManager m_EntityManager{};
		SystemManager m_SystemManager{};
	};
}


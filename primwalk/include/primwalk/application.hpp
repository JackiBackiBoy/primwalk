#pragma once

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/window.hpp"
#include "primwalk/components/entity.hpp"
#include "primwalk/data/model.hpp"
#include "primwalk/managers/componentManager.hpp"
#include "primwalk/managers/entityManager.hpp"
#include "primwalk/managers/systemManager.hpp"
#include "primwalk/rendering/graphicsDevice_Vulkan.hpp"
#include "primwalk/rendering/renderer.hpp"
#include "primwalk/rendering/systems/uiRenderSystem.hpp"
#include "primwalk/systems/renderSystem3d.hpp"
#include "primwalk/ui/GUI.hpp"
#include "primwalk/ui/menuWidget.hpp"
#include "primwalk/ui/subView.hpp"
#include "primwalk/ui/uiElement.hpp"
#include "primwalk/ui/uiEvent.hpp"
#include "primwalk/ui/uiIconButton.hpp"
#include "primwalk/ui/uiImage.hpp"
#include "primwalk/ui/uiLabel.hpp"
#include "primwalk/ui/uiListBox.hpp"
#include "primwalk/ui/uiPanel.hpp"
#include "primwalk/ui/uiSlider.hpp"

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
		virtual ~Application() = default;

		/* Called once at application startup */
		virtual void onStart() = 0;

		/* Called once per frame */
		virtual void onUpdate(float dt) = 0;

		/* Called 60 times per second */
		virtual void onFixedUpdate(float dt) = 0;

		void initialize();
		void run();

		Entity* createEntity(const std::string& name);

	private:
		void gameLoop();
		void updateScene(float dt);
		void renderScene(const FrameInfo& frameInfo);

		std::unique_ptr<Window> m_Window;
		std::unique_ptr<GraphicsDevice> m_Device;
		std::unique_ptr<Renderer> m_Renderer;
		std::unique_ptr<UIRenderSystem> m_UIRenderSystem;
		std::shared_ptr<RenderSystem3D> m_RenderSystem3D;
		std::unique_ptr<SubView> m_SceneView;
		std::unique_ptr<Model> m_Cube;

		// Multi-threading
		std::atomic<bool> m_Resizing { false };
		std::mutex m_RenderingMutex;
		std::condition_variable m_RenderCondition;

		// GUI
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

		// Entities
		std::vector<std::unique_ptr<Entity>> m_Entities{};
		std::shared_ptr<Texture2D> icons = nullptr;

		ComponentManager m_ComponentManager{};
		EntityManager m_EntityManager{};
		SystemManager m_SystemManager{};
	};
}


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
#include "ui/uiEvent.hpp"

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

		std::unique_ptr<Window> m_Window;
		std::unique_ptr<GraphicsDevice> m_Device;
		std::unique_ptr<Renderer> m_Renderer;
		std::unique_ptr<UIRenderSystem> m_UIRenderSystem;
		std::unique_ptr<Model> m_Cube;
		std::unique_ptr<DeferredPass> m_DeferredPass;

		bool m_ScenePaused = false;
		bool m_DebugMode = false;
		bool m_ShowHitboxes = false;

		// Entities
		std::vector<std::unique_ptr<Entity>> m_Entities{};
		std::shared_ptr<Texture2D> icons = nullptr;

		ComponentManager m_ComponentManager{};
		EntityManager m_EntityManager{};
		SystemManager m_SystemManager{};

		friend class Editor;
	};
}


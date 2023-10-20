#pragma once

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/components/component.hpp"
#include "primwalk/managers/componentManager.hpp"
#include "primwalk/managers/entityManager.hpp"
#include "primwalk/managers/systemManager.hpp"

// std
#include <string>
#include <vector>

// vendor
#include <glm/glm.hpp>

namespace pw {
	class PW_API Entity {
	public:
		Entity(const std::string& name,
			ComponentManager& componentManager,
			EntityManager& entityManager,
			SystemManager& systemManager);
		~Entity() = default;

		template <typename T, typename... Args>
		T& addComponent(Args&&... args) {
			auto& component = m_ComponentManager.addComponent<T>(m_ID, std::forward<Args>(args)...);

			component_signature signature = m_EntityManager.getSignature(m_ID);
			signature.set(m_ComponentManager.getComponentType<T>(), true);
			m_EntityManager.setSignature(m_ID, signature);
			m_SystemManager.entitySignatureChanged(m_ID, signature);

			return component;
		}

		template <typename T>
		T& getComponent() {
			return m_ComponentManager.getComponent<T>(m_ID);
		}

	private:
		entity_id m_ID = 0;

		ComponentManager& m_ComponentManager;
		EntityManager& m_EntityManager;
		SystemManager& m_SystemManager;
	};
}
#include "primwalk/components/entity.hpp"

#include "primwalk/components/tag.hpp"
#include "primwalk/components/transform.hpp"
#include "primwalk/components/renderable.hpp"

namespace pw {
	Entity::Entity(const std::string& name,
		ComponentManager& componentManager,
		EntityManager& entityManager,
		SystemManager& systemManager) :
		m_ComponentManager(componentManager),
		m_EntityManager(entityManager),
		m_SystemManager(systemManager) {

		m_ID = entityManager.createEntity();
		addComponent<Tag>().name = name;
		addComponent<Transform>();
		addComponent<Renderable>();
	}
}
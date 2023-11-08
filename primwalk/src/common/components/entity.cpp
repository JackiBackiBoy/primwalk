#include "entity.hpp"

#include "tag.hpp"
#include "transform.hpp"
#include "renderable.hpp"

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
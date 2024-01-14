#include "entity.hpp"

#include "tag.hpp"
#include "transform.hpp"
#include "renderable.hpp"

namespace pw {
	Entity::Entity(const std::string& name,
		ComponentManager& componentManager,
		EntityManager& entityManager) :
		m_ComponentManager(componentManager),
		m_EntityManager(entityManager) {

		m_ID = entityManager.createEntity();
		addComponent<Tag>().name = name;
		addComponent<Transform>();
		addComponent<Renderable>();
	}
}
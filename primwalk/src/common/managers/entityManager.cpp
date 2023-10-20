#include "primwalk/managers/entityManager.hpp"

// std
#include <cassert>

namespace pw {
	EntityManager::EntityManager() {
		for (entity_id e = 0; e < MAX_ENTITIES; e++) {
			m_AvailableEntities.push(e);
		}
	}

	entity_id EntityManager::createEntity() {
		assert(m_LivingEntityCount < MAX_ENTITIES && "ERROR: Too many entities, max number of entities exceeded!");

		entity_id id = m_AvailableEntities.front();
		m_AvailableEntities.pop();
		m_LivingEntityCount++;

		return id;
	}

	void EntityManager::destroyEntity(entity_id entity) {
		assert(entity < MAX_ENTITIES && "ERROR: Entity ID out of range!");

		m_Signatures[entity].reset();
		m_AvailableEntities.push(entity); // destroyed entity at back of queue
		m_LivingEntityCount--;
	}

	component_signature EntityManager::getSignature(entity_id entity) {
		assert(entity < MAX_ENTITIES && "ERROR: Entity ID out of range!");
		return m_Signatures[entity];
	}

	void EntityManager::setSignature(entity_id entity, component_signature signature) {
		assert(entity < MAX_ENTITIES && "ERROR: Entity ID out of range!");
		m_Signatures[entity] = signature;
	}

}
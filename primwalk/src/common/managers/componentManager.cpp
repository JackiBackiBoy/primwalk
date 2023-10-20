#include "primwalk/managers/componentManager.hpp"

namespace pw {

	void ComponentManager::entityDestroyed(entity_id entity) {
		for (const auto& pair : m_ComponentArrays) {
			pair.second->entityDestroyed(entity);
		}
	}

}
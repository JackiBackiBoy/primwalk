#pragma once

// primwalk
#include "../../core.hpp"
#include "../components/component.hpp"

// std
#include <array>
#include <queue>

namespace pw {
	class PW_API EntityManager {
	public:
		EntityManager();
		~EntityManager() = default;

		entity_id createEntity();
		void destroyEntity(entity_id entity);
		component_signature getSignature(entity_id entity);
		void setSignature(entity_id entity, component_signature signature);

	private:
		std::queue<entity_id> m_AvailableEntities;
		std::array<component_signature, MAX_ENTITIES> m_Signatures;

		uint32_t m_LivingEntityCount = 0;
	};
}
#pragma once

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/components/component.hpp"

// std
#include <array>
#include <queue>

namespace pw {
  class PW_API EntityManager {
  public:
    // Remove copy/move constructors
    EntityManager(const EntityManager&) = delete;
    EntityManager(EntityManager&&) = delete;
    EntityManager& operator=(const EntityManager&) = delete;
    EntityManager& operator=(EntityManager&&) = delete;

    static EntityManager& Get();

    entity_id createEntity();
    void destroyEntity(entity_id entity);
    component_signature getSignature(entity_id entity);
    void setSignature(entity_id entity, component_signature signature);

  private:
    EntityManager();
    ~EntityManager() = default;

    std::queue<entity_id> m_AvailableEntities;
    std::array<component_signature, MAX_ENTITIES> m_Signatures;

    uint32_t m_LivingEntityCount = 0;
  };
}
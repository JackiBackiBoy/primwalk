#include "primwalk/managers/componentManager.hpp"

namespace pw {

  ComponentManager& ComponentManager::Get()
  {
    static ComponentManager instance{};
    return instance;
  }

  void ComponentManager::entityDestroyed(entity_id entity)
  {
    for (const auto& pair : m_ComponentArrays) {
      pair.second->entityDestroyed(entity);
    }
  }

}
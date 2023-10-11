#include "primwalk/managers/systemManager.hpp"

namespace pw {
  SystemManager& SystemManager::Get()
  {
    static SystemManager instance{};
    return instance;
  }

  void SystemManager::entityDestroyed(entity_id entity)
  {
    // Erase a destroyed entity from all system lists
    // mEntities is a set so no check needed
    for (auto const& pair : m_Systems)
    {
      auto const& system = pair.second;
      system->entities.erase(entity);
    }
  }

  void SystemManager::entitySignatureChanged(entity_id entity, component_signature signature)
  {
    // Notify each system that an entity's signature changed
    for (auto const& pair : m_Systems)
    {
      auto const& type = pair.first;
      auto const& system = pair.second;
      auto const& systemSignature = m_Signatures[type];

      // Entity signature matches system signature - insert into set
      if ((signature & systemSignature) == systemSignature)
      {
        system->entities.insert(entity);
      }
      // Entity signature does not match system signature - erase from set
      else
      {
        system->entities.erase(entity);
      }
    }
  }

}

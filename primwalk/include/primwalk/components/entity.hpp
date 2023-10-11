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
    Entity() = default;
    Entity(const std::string& name);
    ~Entity() = default;

    template <typename T, typename... Args>
    T& addComponent(Args&&... args) {
      auto& component = ComponentManager::Get().addComponent<T>(m_ID, std::forward<Args>(args)...);

      component_signature signature = EntityManager::Get().getSignature(m_ID);
      signature.set(ComponentManager::Get().getComponentType<T>(), true);
      EntityManager::Get().setSignature(m_ID, signature);
      SystemManager::Get().entitySignatureChanged(m_ID, signature);

      return component;
    }

    template <typename T>
    T& getComponent() {
      return ComponentManager::Get().getComponent<T>(m_ID);
    }

  private:
    entity_id m_ID = 0;
  };
}
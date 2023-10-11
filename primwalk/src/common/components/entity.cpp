#include "primwalk/components/entity.hpp"
#include "primwalk/managers/entityManager.hpp"

#include "primwalk/components/tag.hpp"
#include "primwalk/components/transform.hpp"

namespace pw {
  Entity::Entity(const std::string& name)
  {
    m_ID = EntityManager::Get().createEntity();
    addComponent<Tag>().name = name;
    addComponent<Transform>();
  }
}
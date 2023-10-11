#include "primwalk/data/scene.hpp"

namespace pw {

  void Scene::update()
  {
    for (const auto& e : m_Entities) {

    }
  }

  Entity& Scene::createEntity(const std::string& name)
  {
    m_Entities.emplace_back(name);
    return *(m_Entities.end() - 1);
  }

}
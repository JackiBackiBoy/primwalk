#pragma once

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/components/entity.hpp"

// std
#include <set>
#include <string>
#include <vector>

namespace pw {
  class PW_API Scene {
  public:
    Scene() = default;
    ~Scene() = default;

    void update();

    Entity& createEntity(const std::string& name);

  private:
    std::vector<Entity> m_Entities;
  };
}
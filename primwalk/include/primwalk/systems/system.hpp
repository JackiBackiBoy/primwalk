#pragma once

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/components/component.hpp"

// std
#include <set>

namespace pw {
  class PW_API System {
  public:
    System() = default;
    virtual ~System() = default;

    std::set<entity_id> entities;
  };
}
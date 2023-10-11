#pragma once

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/data/model.hpp"

namespace pw {
  struct PW_API Renderable {
    Model* model = nullptr;
  };
}
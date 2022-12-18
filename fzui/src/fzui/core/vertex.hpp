#pragma once

// FZUI
#include "fzui/core/core.hpp"

// vendor
#include <glm/glm.hpp>

namespace fz {
  struct FZ_API Vertex {
    glm::vec3 position;
    glm::vec2 texCoord;
    float texIndex;
  };
}

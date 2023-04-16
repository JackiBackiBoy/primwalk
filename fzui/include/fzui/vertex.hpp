#pragma once

// FZUI
#include "fzui/core.hpp"

// vendor
#include <glm/glm.hpp>

namespace fz {
  struct FZ_API TextVertex {
    glm::vec3 position{0.0f};
    glm::vec2 texCoord{0.0f};
    float texIndex = 0.0f;
    glm::vec4 color{0.0f};
  };

  struct FZ_API RectVertex {
    glm::vec3 position{0.0f};
    glm::vec2 texCoord{0.0f};
    float texIndex = 0.0f;
    glm::vec4 color{0.0f};
    float borderRadius = 0.0f;
  };
}

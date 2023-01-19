#pragma once

// std
#include <cstdint>

// FZUI
#include "fzui/core/core.hpp"

// vendor
#include <glm/glm.hpp>

namespace fz {
  struct FZ_API Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;

    static glm::vec3 normalize(const Color& color);
  };
}

#pragma once

// std
#include <cstdint>

// FZUI
#include "fzui/core.hpp"

// vendor
#include <glm/glm.hpp>

namespace fz {
  struct FZ_API Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a = 255;

    static glm::vec4 normalize(const Color& color);
  };
}

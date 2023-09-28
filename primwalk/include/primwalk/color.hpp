#pragma once

// std
#include <cstdint>

// primwalk
#include "primwalk/core.hpp"

// vendor
#include <glm/glm.hpp>

namespace pw {
  struct PW_API Color {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;

    static glm::vec4 normalize(const Color& color);

    // Predefined colors
    static Color White;
    static Color Black;
  };
}


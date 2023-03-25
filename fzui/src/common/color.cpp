#include "fzui/color.hpp"

namespace fz {
  glm::vec4 Color::normalize(const Color& color) {
    return { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f };
  }
}

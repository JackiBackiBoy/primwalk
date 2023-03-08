#include "fzui/windows/color.hpp"

namespace fz {
  glm::vec3 Color::normalize(const Color& color) {
    return { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f };
  }

  const Color Color::White = { 255, 255, 255 };
}

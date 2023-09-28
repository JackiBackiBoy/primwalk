#include "primwalk/color.hpp"

namespace pw {
  glm::vec4 Color::normalize(const Color& color) {
    return { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f };
  }

  // Predefined colors
  Color Color::White = { 255, 255, 255 };
  Color Color::Black = { 0, 0, 0 };
}

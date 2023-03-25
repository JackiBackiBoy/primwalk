// FZUI
#include "fzui/math/math.hpp"

// std
#include <algorithm>

namespace fz {
  float Math::lerp(const float& a, const float& b, float percentage) {
    percentage = std::clamp(percentage, 0.0f, 1.0f); // percentage must be in range [0, 1]
    float diff = b - a;
    return a + diff * percentage;
  }
}

#pragma once

// FZUI
#include "fzui/core/core.hpp"

namespace fz {
  struct FZ_API FontCharacter {
    int id = 0;
    double sizeX = 0.0;
    double sizeY = 0.0;
    double xOffset = 0.0;
    double yOffset = 0.0;
    double xAdvance = 0.0;
    double xLeftTexCoord = 0.0;
    double yBottomTexCoord = 0.0;
    double xRightTexCoord = 0.0;
    double yTopTexCoord = 0.0;
  };
}

#pragma once

#include "fzui/core/core.hpp"

namespace fz {
  enum class Win32HoverFlags {
    None = -1,
    Unhovered = 0, // should only be used if element has already been hovered
    Hovering = 1,
  };
}

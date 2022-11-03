#pragma once

// std
#include <string>

// FZUI
#include "fzui/core/core.hpp"
#include "fzui/color.hpp"
#include "fzui/ui/uiStyle.hpp"

namespace fz {
  struct FZ_API WindowInfo {
    int titleBarHeight = 29;
    Color titleBarColor = UiStyle::darkCaptionColor;
    Color backgroundColor = UiStyle::darkBackground;

    static WindowInfo DefaultDark;
  };
}

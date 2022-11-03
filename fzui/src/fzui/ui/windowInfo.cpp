#include "fzui/ui/windowInfo.hpp"

namespace fz {
  WindowInfo WindowInfo::DefaultDark = {
    .titleBarHeight = 29,
    .titleBarColor = UiStyle::darkCaptionColor,
    .backgroundColor = UiStyle::darkBackground,
  };
}

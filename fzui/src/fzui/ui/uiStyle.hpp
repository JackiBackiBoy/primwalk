#pragma once

#include "fzui/core/core.hpp"
#include "fzui/color.hpp"

namespace fz {
  class FZ_API UiStyle {
    public:
      // ------ General ------
      // Client Area
      static constexpr Color darkBackground = { 39, 39, 39 };
      static constexpr Color lightBackground = { 255, 255, 255 };

      // Caption area (non-client)
      static constexpr Color darkCaptionColor = { 60, 60, 60 };
      static constexpr int defaultMenuHeight = 22;

      // ------ Buttons ------
      static constexpr Color darkButtonDefaultColor = { 57, 57, 57 };
      static constexpr Color darkButtonHoverColor = { 87, 87, 87 };
      static constexpr Color darkButtonClickColor = { 100, 100, 100 };
      static constexpr Color darkButtonBorderColor = { 120, 120, 120 };

      static constexpr Color lightButtonColor = { 240, 240, 240 };
      static constexpr Color lightButtonHoverColor = { 50, 50, 240 };

    private:
      UiStyle() {};
      ~UiStyle() {};
  };
}

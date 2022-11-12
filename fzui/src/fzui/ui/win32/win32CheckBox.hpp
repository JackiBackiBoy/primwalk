#pragma once

// std
#include <string>

#include "fzui/core/core.hpp"
#include "fzui/ui/win32/win32UiElement.hpp"

namespace fz {
  class FZ_API Win32CheckBox : public Win32UiElement {
    public:
      Win32CheckBox() : Win32UiElement() {};

      CREATESTRUCT getCreateStruct() override;
      std::string getTypeString() const override;

    private:
  };
}

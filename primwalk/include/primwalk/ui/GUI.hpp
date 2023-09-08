#pragma once

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/ui/uiElement.hpp"
#include "primwalk/ui/uiEvent.hpp"
#include "primwalk/rendering/systems/uiRenderSystem.hpp"

// std
#include <memory>
#include <vector>

namespace pw::gui {
  bool processEvent(const UIEvent& event);

  class PW_API GUI {
  public:
    GUI() {};
    ~GUI() = default;

    void addWidget(UIElement* widget);

    void onUpdate();
    void onRender(UIRenderSystem& renderer);

  private:
    
  };
}
#pragma once

// primwalk
#include "primwalk/color.hpp"
#include "primwalk/core.hpp"
#include "primwalk/ui/uiElement.hpp"
#include "primwalk/ui/uiEvent.hpp"

namespace pw {
  class PW_API UISlider : public UIElement {
  public:
    UISlider() : UIElement(glm::vec2(0), true) {};
    virtual ~UISlider() = default;

    virtual void onRender(UIRenderSystem& renderer) override;
    virtual void handleEvent(const UIEvent& event) override;
    virtual Hitbox hitboxTest(glm::vec2 position) override;

    inline float getSliderValue() const { return m_CurrentVal; }

    inline void setTrackColor(Color color) { m_TrackColor = color; }
    inline void setSliderColor(Color color) { m_SliderColor = color; }

  private:
    int m_Width = 100;
    float m_MinVal = 0;
    float m_MaxVal = 100;
    float m_DefaultVal = 50;
    float m_CurrentVal = 50;
    int m_TrackHeight = 4;
    int m_SliderRadius = 6;

    bool m_Hovered = false;
    bool m_Pressed = false;

    Color m_TrackColor = { 128, 128, 128 };
    Color m_TrackHoverColor = m_TrackColor;
    Color m_SliderColor = { 255, 255, 255 };
    Color m_SliderHoverColor = { 0, 156, 255 };
    
    Color m_DisplayTrackColor = m_TrackColor;
    Color m_DisplaySliderColor = m_SliderColor;
  };
}


#pragma once

#include "primwalk/color.hpp"
#include "primwalk/core.hpp"
#include "primwalk/ui/uiElement.hpp"

namespace pw {
  class PW_API UISlider : public UIElement {
  public:
    UISlider(glm::vec2 position, int width, float minVal, float maxVal, float defaultVal);
    virtual ~UISlider() = default;

    virtual void onRender(UIRenderSystem& renderer) override;
    virtual void handleEvent(const UIEvent& event) override;
    virtual Hitbox hitboxTest(glm::vec2 position) override;

    virtual Hitbox getHitbox() override;
    inline float getSliderValue() const { return m_CurrentVal; }

    inline void setTrackColor(Color color) { m_TrackColor = color; }
    inline void setSliderColor(Color color) { m_SliderColor = color; }

  private:
    int m_Width;
    float m_MinVal;
    float m_MaxVal;
    float m_DefaultVal;
    float m_CurrentVal;
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
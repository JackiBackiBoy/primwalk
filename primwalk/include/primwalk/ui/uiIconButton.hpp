#ifndef PW_UI_ICON_BUTTON_HEADER
#define PW_UI_ICON_BUTTON_HEADER

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/color.hpp"
#include "primwalk/ui/uiElement.hpp"
#include "primwalk/rendering/texture2D.hpp"

// std
#include <memory>

// vendor
#include <glm/glm.hpp>

namespace pw {
  class PW_API UIIconButton : public UIElement {
  public:
    UIIconButton();
    virtual ~UIIconButton() {};

    virtual void onRender(UIRenderSystem& renderer) override;
    virtual void handleEvent(const UIEvent& event) override;
    virtual Hitbox hitboxTest(glm::vec2 position) override;

    // Getters
    inline int getWidth() const { return m_Width; }
    inline int getHeight() const { return m_Height; }
    virtual Hitbox getHitbox() override;

    // Setters
    inline void setWidth(int width) { m_Width = width; }
    inline void setHeight(int height) { m_Height = height; }
    inline void setBackgroundColor(Color color) { m_BackgroundColor = color; }
    inline void setIconColor(Color color) { m_IconColor = color; }
    inline void setBackgroundHoverColor(Color color) { m_BackgroundHoverColor = color; }
    inline void setBackgroundClickColor(Color color) { m_BackgroundClickColor = color; }
    inline void setIcon(std::shared_ptr<Texture2D> icon) { m_Icon = icon; }

  private:
    int m_Width = 0;
    int m_Height = 0;
    std::shared_ptr<Texture2D> m_Icon = nullptr;
    bool m_Hovered = false;
    bool m_Pressed = false;

    Color m_BackgroundColor = { 255, 255, 255, 0 }; // transparent by default
    Color m_BackgroundHoverColor = { 128, 128, 128 };
    Color m_BackgroundClickColor = { 128, 128, 128 };
    Color m_BackgroundDisplayColor = m_BackgroundColor;
    Color m_IconColor = { 255, 255, 255 }; // white by default (multiplicative color)
  };
}

#endif
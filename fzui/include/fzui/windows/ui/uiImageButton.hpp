#ifndef FZ_UI_IMAGE_BUTTON_HEADER
#define FZ_UI_IMAGE_BUTTON_HEADER

// FZUI
#include "fzui/core.hpp"
#include "fzui/windows/ui/uiElement.hpp"
#include "fzui/windows/color.hpp"

namespace fz {
  class FZ_API UIImageButton : public UIElement {
    public:
      UIImageButton(Texture* texture, const glm::vec2& pos, const int& width, const int& height);
      virtual ~UIImageButton() = default;

      virtual void update(const float& dt) override;
      virtual void draw(Renderer2D* renderer) override;

      // Setters
      void setBackgroundColor(const Color& color);
      void setHoverColor(const Color& color);

    private:
      Texture* m_Texture;
      int m_Width;
      int m_Height;
      Color m_BackgroundColor;
      Color m_HoverColor;
      Color m_DisplayColor;
      bool m_IsHovered = false;
      float m_Timer = 0.0f;
      float m_HoverTransition = 0.5f;
  };
}
#endif
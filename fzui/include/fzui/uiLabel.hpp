#ifndef FZ_UI_LABEL_HEADER
#define FZ_UI_LABEL_HEADER

// FZUI
#include "fzui/core.hpp"
#include "fzui/uiElement.hpp"
#include "fzui/color.hpp"

namespace fz {
  class FZ_API UILabel : public UIElement {
    public:
      UILabel(const std::string& text, const glm::vec2& pos, const Color& color, const float& fontSize, FontFace* font = nullptr);
      virtual ~UILabel() = default;

      virtual void update(const float& dt) override;
      virtual void draw(Renderer2D* renderer) override;

      // Setters

    private:
      std::string m_Text = "";
      Color m_TextColor = { 255, 255, 255 };
      float m_FontSize = 0.0f;
      FontFace* m_Font = nullptr;
  };
}

#endif

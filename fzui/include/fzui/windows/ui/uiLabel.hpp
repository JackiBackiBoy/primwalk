#ifndef FZ_UI_LABEL_HEADER
#define FZ_UI_LABEL_HEADER

// FZUI
#include "fzui/core.hpp"
#include "fzui/windows/ui/uiElement.hpp"
#include "fzui/windows/color.hpp"

namespace fz {
  class FZ_API UILabel : public UIElement {
    public:
      UILabel(const std::string& text, const glm::vec2& pos, const Color& color, const float& fontSize, FontFace* font = nullptr);
      virtual ~UILabel() = default;

      virtual void update(const float& dt) override;
      virtual void draw(Renderer2D* renderer) override;

    private:
      std::string m_Text;
      Color m_TextColor;
      float m_FontSize;
      FontFace* m_Font;
  };
}

#endif
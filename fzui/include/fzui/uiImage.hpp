#ifndef FZ_UI_IMAGE_HEADER
#define FZ_UI_IMAGE_HEADER

// FZUI
#include "fzui/core.hpp"
#include "fzui/uiElement.hpp"
#include "fzui/color.hpp"
#include "fzui/data/texture.hpp"

namespace fz {
  class FZ_API UIImage : public UIElement {
    public:
      UIImage(Texture* texture, const glm::vec2& pos, const int& width, const int& height);
      virtual ~UIImage() = default;

      virtual void update(const float& dt) override;
      virtual void draw(Renderer2D* renderer) override;

    private:
      std::string m_Text;
      Color m_Color;
      Texture* m_Texture = nullptr;
      int m_Width;
      int m_Height;
  };
}

#endif

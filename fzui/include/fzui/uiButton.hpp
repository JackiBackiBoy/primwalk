#ifndef FZ_UI_BUTTON_HEADER
#define FZ_UI_BUTTON_HEADER

// FZUI
#include "fzui/core.hpp"
#include "fzui/color.hpp"
#include "fzui/uiElement.hpp"

// std
#include <string>

namespace fz {
  class FZ_API UIButton : public UIElement {
    public:
      UIButton(const std::string& text, glm::vec2 position, int width, int height) :
        UIElement(position), m_Text(text), m_Width(width), m_Height(height) {};
      virtual ~UIButton() {};

      virtual void onRender(UIRenderSystem& renderer);

      // Setters
      void setText(const std::string& text);
      void setBackgroundColor(Color color);

    private:
      std::string m_Text;
      int m_Width;
      int m_Height;

      // Colors
      Color m_BackgroundColor = { 255, 255, 255 };
  };
}
#endif
#ifndef FZ_UI_LABEL_HEADER
#define FZ_UI_LABEL_HEADER

// FZUI
#include "fzui/core.hpp"
#include "fzui/color.hpp"
#include "fzui/uiElement.hpp"

// std
#include <string>

namespace fz {
  class FZ_API UILabel : public UIElement {
  public:
    UILabel(const std::string& text, glm::vec2 position) :
      UIElement(position), m_Text(text) {};
    virtual ~UILabel() {};

    virtual void onRender(UIRenderSystem& renderer);

    // Setters
    void setText(const std::string& text);
    void setBackgroundColor(Color color);

  private:
    std::string m_Text;

    // Colors
    Color m_BackgroundColor = { 255, 255, 255 };
  };
}
#endif
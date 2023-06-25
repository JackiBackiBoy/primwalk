#ifndef FZ_UI_LABEL_HEADER
#define FZ_UI_LABEL_HEADER

// FZUI
#include "fzui/core.hpp"
#include "fzui/color.hpp"
#include "fzui/ui/uiElement.hpp"
#include "fzui/data/font.hpp"

// std
#include <memory>
#include <string>

namespace fz {
  class FZ_API UILabel : public UIElement {
  public:
    UILabel(const std::string& text, glm::vec2 position, std::shared_ptr<Font> font = nullptr);
    virtual ~UILabel() {};

    virtual void onRender(UIRenderSystem& renderer) override;
    virtual void handleEvent(const UIEvent& event) override;
    virtual Hitbox hitboxTest(glm::vec2 position) override;

    // Getters
    virtual Hitbox getHitbox() override;

    // Setters
    void setText(const std::string& text);
    void setBackgroundColor(Color color);
    inline void setTextColor(Color color) { m_TextColor = color; }
    inline void setFontSize(double size) { m_FontSize = size; }

  private:
    std::string m_Text;
    std::shared_ptr<Font> m_Font = nullptr;
    double m_FontSize = 0;

    // Colors
    Color m_BackgroundColor = { 255, 255, 255 };
    Color m_TextColor = { 255, 255, 255 };
  };
}
#endif
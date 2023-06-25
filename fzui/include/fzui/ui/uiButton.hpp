#ifndef FZ_UI_BUTTON_HEADER
#define FZ_UI_BUTTON_HEADER

// FZUI
#include "fzui/core.hpp"
#include "fzui/color.hpp"
#include "fzui/ui/uiElement.hpp"
#include "fzui/managers/resourceManager.hpp"

// std
#include <memory>
#include <string>
#include <mutex>

namespace fz {
  class FZ_API UIButton : public UIElement {
    public:
      UIButton(const std::string& text, glm::vec2 position, int width, int height, std::shared_ptr<Font> font = nullptr) :
        UIElement(position), m_Text(text), m_Width(width), m_Height(height), m_Font(font)
      {
        if (font == nullptr) {
          m_Font = ResourceManager::Get().findFont("Catamaran", FontWeight::Bold);
        }
      };
      virtual ~UIButton() {};

      virtual void onRender(UIRenderSystem& renderer) override;
      virtual void handleEvent(const UIEvent& event) override;
      virtual Hitbox hitboxTest(glm::vec2 position) override;

      // Getters
      virtual Hitbox getHitbox() override;

      // Setters
      inline void setText(const std::string& text) { m_Text = text; }
      void setBackgroundColor(Color color);
      inline void setBackgroundHoverColor(Color color) { m_BackgroundHoverColor = color; }
      inline void setBackgroundClickColor(Color color) { m_BackgroundClickColor = color; }
      inline void setTextColor(Color color) { m_TextColor = color; }
      inline void setTextHoverColor(Color color) { m_TextHoverColor = color; }
      inline void setBorderRadius(uint32_t radius) { m_BorderRadius = radius; }
      inline void setIcon(std::shared_ptr<Texture2D> texture) { m_Icon = texture; }

    private:
      std::string m_Text;
      std::string m_FontName;
      int m_Width;
      int m_Height;
      uint32_t m_BorderRadius = 0;
      bool m_Hovered = false;
      bool m_Pressed = false;
      std::mutex m_PropertyMutex;
      std::shared_ptr<Texture2D> m_Icon = nullptr;
      std::shared_ptr<Font> m_Font = nullptr;

      // Colors
      Color m_BackgroundColor = { 255, 255, 255 };
      Color m_BackgroundHoverColor = { 128, 128, 128 };
      Color m_BackgroundClickColor = { 128, 128, 128 };
      Color m_TextColor = { 0, 0, 0 };
      Color m_TextHoverColor = { 0, 0, 0 };
      Color m_DisplayColor = m_BackgroundColor;
  };
}
#endif
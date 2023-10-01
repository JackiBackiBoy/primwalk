#pragma once

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/color.hpp"
#include "primwalk/ui/uiElement.hpp"
#include "primwalk/data/font.hpp"

// std
#include <memory>
#include <string>
#include <vector>

namespace pw {
  class MenuWidget;

  class PW_API MenuItem {
  public:
    MenuItem() = default;
    ~MenuItem() = default;

    void addItem(MenuItem* item);
    void addSeparator();

    inline std::string getText() const { return m_Text; }
    bool hitboxTest(glm::vec2 position) const;
    MenuItem* menuHitTest(glm::vec2 position);
    glm::vec2 getAbsolutePosition() const;
    MenuItem* getTopParent();

    inline void setText(const std::string& text) { m_Text = text; }

  private:
    void* m_Parent = nullptr;
    bool m_HasMenuParent = false;
    std::vector<MenuItem*> m_ChildItems;
    std::string m_Text = "";
    glm::vec2 m_Position = { 0, 0 };
    float m_Width = 0;
    float m_ItemHeight = 20;
    float m_ItemMargin = 10;
    float m_FontSize = 12;
    int m_MaxChildWidth = 0;
    size_t m_MenuDepth = 0;
    std::shared_ptr<Font> m_Font = nullptr;

    friend class MenuWidget;
  };

  class PW_API MenuWidget : public UIElement {
  public:
    MenuWidget();
    virtual ~MenuWidget() = default;

    void addItem(MenuItem* item);

    void onRender(UIRenderSystem& renderer) override;
    void handleEvent(const UIEvent& event) override;
    Hitbox hitboxTest(glm::vec2 position) override;
    MenuItem* menuHitTest(glm::vec2 position);

  private:
    std::vector<MenuItem*> m_Items;
    std::shared_ptr<Font> m_Font = nullptr;
    MenuItem* m_CurrentItem = nullptr;

    // Colors
    Color m_BackgroundColor = { 31, 31, 31 };
    Color m_BackgroundHoverColor = { 61, 61, 61 };
    Color m_BackgroundClickColor = m_BackgroundHoverColor;
    Color m_DisplayBackgroundColor = m_BackgroundColor;

    Color m_BorderColor = { 100, 100, 100 };
    Color m_BorderHoverColor = { 112, 112, 112 };
    Color m_BorderClickcolor = { 150, 150, 150 };
    Color m_DisplayBorderColor = m_BorderColor;

    float m_ItemHeight = 20;
    float m_ItemMargin = 10;
    float m_Width = 0;
    float m_FontSize = 12;
    int m_MenuDepth = 0;
  };
}
#ifndef FZ_UI_CONTAINER_HEADER
#define FZ_UI_CONTAINER_HEADER

// std
#include <vector>

// vendor
#include <glm/glm.hpp>

// FZUI
#include "fzui/core.hpp"
#include "fzui/color.hpp"
#include "fzui/uiElement.hpp"
#include "fzui/rendering/renderer2d.hpp"

namespace fz {
  class FZ_API UIContainer {
  public:
    UIContainer(int width, int height, const glm::vec2& position);
    ~UIContainer() {};

    void addElement(UIElement* element);
    void addContainer(UIContainer* container);
    void onUpdate(float dt);
    void onRender(Renderer2D* renderer);

    // Getters
    glm::vec2 getAbsolutePosition();

    // Setters
    void setHeight(int height);
    void setPosition(const glm::vec2& position);
    void setBorderRadius(int radius);
    void setBackgroundColor(const Color& color);

  private:
    std::vector<UIElement*> m_Elements;
    std::vector<UIContainer*> m_Containers;
    glm::vec2 m_Position = { 0, 0 };
    int m_Width = 0;
    int m_Height = 0;
    int m_BorderRadius = 0;
    UIContainer* m_Parent = nullptr;

    // Colors
    Color m_BackgroundColor = { 0, 0, 0 };
    Color m_HoverColor = { 0, 0, 0 };
    Color m_DisplayColor = { 0, 0, 0 };

    // Flags
    bool m_IsHovered = false;
    float m_Timer = 0.0f;
    float m_HoverTransition = 0.5f;
  };
}
#endif
#include "fzui/uiContainer.hpp"
#include "fzui/mouse.hpp"
#include "fzui/math/math.hpp"

namespace fz {

  UIContainer::UIContainer(int width, int height, const glm::vec2& position)
  {
    m_Width = width;
    m_Height = height;
    m_Position = position;
    m_HoverColor = { 200, 200, 200 };
    m_DisplayColor = m_BackgroundColor;
    m_HoverTransition = 0.5f; // seconds
  }

  void UIContainer::addElement(UIElement* element)
  {
    element->m_Container = this;
    m_Elements.push_back(element);
  }

  void UIContainer::addContainer(UIContainer* container)
  {
    container->m_Parent = this;
    m_Containers.push_back(container);
  }

  void UIContainer::onUpdate(float dt)
  {
    glm::vec2 mousePos = Mouse::Instance().getRelativePos();
    glm::vec2 truePos = getAbsolutePosition();

    // Collision detection
    if (mousePos.x >= truePos.x && mousePos.x <= truePos.x + m_Width &&
        mousePos.y >= truePos.y && mousePos.y <= truePos.y + m_Height) {
      m_IsHovered = true;
      m_Timer = std::clamp(m_Timer + dt, 0.0f, m_HoverTransition);
    }
    else {
      m_IsHovered = false;
      m_Timer = std::clamp(m_Timer - dt, 0.0f, m_HoverTransition);
    }

    // Hover and unhover animation
    float percentage = m_Timer / m_HoverTransition;
    m_DisplayColor.r = Math::lerp(m_BackgroundColor.r, m_HoverColor.r, percentage);
    m_DisplayColor.g = Math::lerp(m_BackgroundColor.g, m_HoverColor.g, percentage);
    m_DisplayColor.b = Math::lerp(m_BackgroundColor.b, m_HoverColor.b, percentage);
    m_DisplayColor.a = Math::lerp(m_BackgroundColor.a, m_HoverColor.a, percentage);

    for (UIElement* e : m_Elements) {
      e->update(dt);
    }

    for (UIContainer* c : m_Containers) {
      c->onUpdate(dt);
    }
  }

  void UIContainer::onRender(Renderer2D* renderer)
  {
    renderer->drawRect(m_Width, m_Height, getAbsolutePosition(), m_DisplayColor, m_BorderRadius);

    for (UIElement* e : m_Elements) {
      e->draw(renderer);
    }

    for (UIContainer* c : m_Containers) {
      c->onRender(renderer);
    }
  }

  // Getters
  glm::vec2 UIContainer::getAbsolutePosition()
  {
    if (m_Parent != nullptr) {
      return m_Position + m_Parent->getAbsolutePosition();
    }

    return m_Position;
  }

  void UIContainer::setHeight(int height)
  {
    m_Height = height;
  }

  // Setters
  void UIContainer::setPosition(const glm::vec2& position)
  {
    m_Position = position;
  }

  void UIContainer::setBorderRadius(int radius)
  {
    m_BorderRadius = radius;
  }

  void UIContainer::setBackgroundColor(const Color& color)
  {
    m_BackgroundColor = color;
  }

}

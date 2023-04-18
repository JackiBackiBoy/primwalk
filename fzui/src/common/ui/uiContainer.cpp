#include "fzui/uiContainer.hpp"

namespace fz {

  UIContainer::UIContainer(int width, int height, const glm::vec2& position)
  {
    m_Width = width;
    m_Height = height;
    m_Position = position;
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
    for (UIElement* e : m_Elements) {
      e->update(dt);
    }

    for (UIContainer* c : m_Containers) {
      c->onUpdate(dt);
    }
  }

  void UIContainer::onRender(Renderer2D* renderer)
  {
    renderer->drawRect(m_Width, m_Height, getAbsolutePosition(), m_BackgroundColor, m_BorderRadius);

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

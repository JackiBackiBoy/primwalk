#include "primwalk/ui/menuWidget.hpp"
#include "primwalk/managers/resourceManager.hpp"

// std
#include <algorithm>

namespace pw {

  void MenuItem::addItem(MenuItem* item)
  {
    item->m_Parent = this;
    item->m_MenuDepth = m_MenuDepth + 1;
    item->m_Position = m_Position + glm::vec2(0, (m_ChildItems.size() + 1) * m_ItemHeight);
    item->m_Font = ResourceManager::Get().findFont("Motiva Sans", FontWeight::Regular);
    item->m_Width = 2 * m_ItemMargin + item->m_Font->getTextWidth(item->getText(), item->m_FontSize);

    m_MaxChildWidth = std::max(m_MaxChildWidth, (int)item->m_Width);

    m_ChildItems.push_back(item);
  }

  void MenuItem::addSeparator()
  {
    
  }

  // ------ Menu Item ------
  bool MenuItem::hitboxTest(glm::vec2 position) const
  {
    Hitbox hitbox = Hitbox(getAbsolutePosition(), m_Width, m_ItemHeight, nullptr);

    if (hitbox.contains(position)) {
      return true;
    }

    for (const auto m : m_ChildItems) {
      if (m->hitboxTest(position)) {
        return true;
      }
    }

    return false;
  }

  MenuItem* MenuItem::menuHitTest(glm::vec2 position)
  {
    for (const auto m : m_ChildItems) {
      if (m->hitboxTest(position)) {
        return m->menuHitTest(position);
      }
    }

    if (hitboxTest(position)) {
      return this;
    }

    return nullptr;
  }

  glm::vec2 MenuItem::getAbsolutePosition() const
  {
    std::string text = m_Text;
    glm::vec2 position = m_Position;

    if (!m_HasMenuParent) {
      position += reinterpret_cast<MenuItem*>(m_Parent)->getAbsolutePosition();
    }
    else {
      position += reinterpret_cast<MenuWidget*>(m_Parent)->getAbsolutePosition();
    }

    return position;
  }


  MenuItem* MenuItem::getTopParent()
  {
    if (!m_HasMenuParent) {
      return reinterpret_cast<MenuItem*>(m_Parent)->getTopParent();
    }

    return this;
  }

  // ------ Menu Widget ------
  MenuWidget::MenuWidget()
  {
    
  }

  void MenuWidget::addItem(MenuItem* item)
  {
    if (m_Font == nullptr) {
      m_Font = ResourceManager::Get().findFont("Motiva Sans", FontWeight::Regular);
    }

    float itemWidth = 2 * m_ItemMargin + m_Font->getTextWidth(item->getText(), m_FontSize);
    item->m_Width = itemWidth;
    item->m_Position = { m_Width, 0.0f};
    item->m_Parent = this;
    item->m_HasMenuParent = true;
    item->m_ItemHeight = m_ItemHeight;

    m_Items.push_back(item);
    m_Width += itemWidth;
  }

  void MenuWidget::onRender(UIRenderSystem& renderer)
  {
    renderer.drawRect(getAbsolutePosition(), m_Width, m_ItemHeight, m_BackgroundColor);

    glm::vec2 itemPos = getAbsolutePosition();
    for (const auto m : m_Items) {
      float textWidth = m_Font->getTextWidth(m->getText(), m_FontSize);
      float itemWidth = 2 * m_ItemMargin + textWidth;

      renderer.drawText(itemPos + glm::vec2(itemWidth / 2 - textWidth / 2,
        m_ItemHeight / 2 - (m_Font->getMaxHeight() * (m_FontSize / m_Font->getFontSize())) / 2),
        m->getText(), m_FontSize, Color::White, m_Font);
      itemPos.x += itemWidth;
    }

    if (m_CurrentItem != nullptr) {
      if (m_MenuDepth > 0) {
        MenuItem* menuHead = m_CurrentItem->getTopParent();
        int depth = 0;

        while (depth < m_MenuDepth) {
          for (size_t i = 0; i < menuHead->m_ChildItems.size(); i++) {
            float textWidth = m_Font->getTextWidth(menuHead->m_ChildItems[i]->getText(), m_FontSize);
            float itemWidth = menuHead->m_MaxChildWidth;

            renderer.drawRect(menuHead->getAbsolutePosition() + glm::vec2(0, m_ItemHeight * (i + 1)),
              itemWidth, m_ItemHeight, m_BackgroundColor);
            renderer.drawText(menuHead->getAbsolutePosition() +
              glm::vec2(0, m_ItemHeight * (i + 1)) + glm::vec2(itemWidth / 2 - textWidth / 2,
              m_ItemHeight / 2 - (m_Font->getMaxHeight() * (m_FontSize / m_Font->getFontSize())) / 2),
              menuHead->m_ChildItems[i]->getText(), m_FontSize, Color::White, m_Font);

            if (!menuHead->m_ChildItems[i]->m_ChildItems.empty()) {
              menuHead = menuHead->m_ChildItems[i];
            }
          }

          depth++;
        }
      }

      renderer.drawRect(m_CurrentItem->getAbsolutePosition(), m_CurrentItem->m_Width, m_ItemHeight, m_BorderHoverColor);
      renderer.drawRect(m_CurrentItem->getAbsolutePosition() + glm::vec2(1), m_CurrentItem->m_Width - 2, m_ItemHeight - 2, m_BackgroundHoverColor);
    }
  }

  void MenuWidget::handleEvent(const UIEvent& event)
  {
    switch (event.getType()) {
    case UIEventType::MouseEnter:
    case UIEventType::MouseMove:
      {
        m_CurrentItem = menuHitTest(event.getMouseData().position);
      }
      break;
    case UIEventType::MouseExit:
      m_MenuDepth = 0;
      m_CurrentItem = nullptr;
      m_DisplayBackgroundColor = m_BackgroundColor;
      break;
    case UIEventType::MouseDown:
      {
        if (m_CurrentItem != nullptr) {
          m_MenuDepth = 1;
        }
      }
      break;
    }
  }

  // TODO: Returning a hitbox is stupid, just return true or false
  Hitbox MenuWidget::hitboxTest(glm::vec2 position)
  {
    Hitbox hitbox(getAbsolutePosition(), m_Width, m_ItemHeight, this);

    if (hitbox.contains(position)) {
      return hitbox;
    }

    if (m_MenuDepth > 0) {
      bool hitSubItem = false;
      for (const auto m : m_Items) {
        hitSubItem = m->hitboxTest(position);

        if (hitSubItem) {
          break;
        }
      }

      if (hitSubItem) {
        return hitbox;
      }
    }

    return Hitbox(getAbsolutePosition(), m_Width, m_ItemHeight, nullptr);
  }

  MenuItem* MenuWidget::menuHitTest(glm::vec2 position)
  {
    for (const auto m : m_Items) {
      bool hitSubItem = m->hitboxTest(position);

      if (hitSubItem) {
        return m->menuHitTest(position);
      }
    }

    return nullptr;
  }
}
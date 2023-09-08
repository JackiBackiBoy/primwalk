#include "primwalk/ui/menuWidget.hpp"
#include "primwalk/managers/resourceManager.hpp"
#include <iostream>

namespace pw {

  void MenuItem::addItem(MenuItem* item)
  {
    item->m_Parent = this;
    item->m_MenuDepth = m_MenuDepth + 1;
    item->m_Position = m_Position + glm::vec2(0, (m_ChildItems.size() + 1) * m_ItemHeight);
    item->m_Font = ResourceManager::Get().findFont("Catamaran", FontWeight::Bold);
    item->m_Width = 2 * m_ItemMargin + item->m_Font->getTextWidth(item->getText(), item->m_Font->getFontSize());

    m_ChildItems.push_back(item);
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


  // ------ Menu Widget ------
  MenuWidget::MenuWidget()
  {
    
  }

  void MenuWidget::addItem(MenuItem* item)
  {
    if (m_Font == nullptr) {
      m_Font = ResourceManager::Get().findFont("Catamaran", FontWeight::Bold);
    }

    float itemWidth = 2 * m_ItemMargin + m_Font->getTextWidth(item->getText(), m_Font->getFontSize());
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
      float textWidth = m_Font->getTextWidth(m->getText(), m_Font->getFontSize());
      float itemWidth = 2 * m_ItemMargin + textWidth;

      renderer.drawText(itemPos + glm::vec2(itemWidth / 2 - textWidth / 2, m_ItemHeight / 2 - m_Font->getMaxHeight() / 2), m->getText(), 0, Color::White);
      itemPos.x += itemWidth;
    }

    if (m_CurrentItem != nullptr) {
      renderer.drawRect(m_CurrentItem->getAbsolutePosition(), m_CurrentItem->m_Width, m_ItemHeight, m_BorderHoverColor);
      renderer.drawRect(m_CurrentItem->getAbsolutePosition() + glm::vec2(1), m_CurrentItem->m_Width - 2, m_ItemHeight - 2, m_DisplayBackgroundColor);

      if (m_MenuDepth > 0) {
        MenuItem* menuHead = m_CurrentItem;
        int depth = 0;

        do {
          for (size_t i = 0; i < menuHead->m_ChildItems.size(); i++) {
            float textWidth = m_Font->getTextWidth(menuHead->m_ChildItems[i]->getText(), m_Font->getFontSize());
            float itemWidth = 2 * m_ItemMargin + textWidth;

            renderer.drawRect(menuHead->getAbsolutePosition() + glm::vec2(0, m_ItemHeight * (i + 1)), itemWidth, m_ItemHeight, m_BackgroundColor);
            renderer.drawText(menuHead->getAbsolutePosition() + glm::vec2(0, m_ItemHeight * (i + 1)) + glm::vec2(itemWidth / 2 - textWidth / 2,
              m_ItemHeight / 2 - m_Font->getMaxHeight() / 2), menuHead->m_ChildItems[i]->getText(), 0, Color::White);

            if (!menuHead->m_ChildItems[i]->m_ChildItems.empty()) {
              menuHead = menuHead->m_ChildItems[i];
            }
          }

          depth++;
        } while (depth < m_MenuDepth);
      }
    }
  }

  void MenuWidget::handleEvent(const UIEvent& event)
  {
    switch (event.getType()) {
    case UIEventType::MouseEnter:
    case UIEventType::MouseMove:
      {
        glm::vec2 mousePos = event.getMouseData().position;
        
        for (const auto m : m_Items) {
          if (m->hitboxTest(mousePos)) {
            m_CurrentItem = m;
            m_DisplayBackgroundColor = m_BackgroundHoverColor;
            break;
          }
        }
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
    Hitbox hitbox = getHitbox();

    if (hitbox.contains(position)) {
      return hitbox;
    }

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

    return Hitbox(getAbsolutePosition(), m_Width, m_ItemHeight, nullptr);
  }

  Hitbox MenuWidget::getHitbox()
  {
    return Hitbox(getAbsolutePosition(), m_Width, m_ItemHeight, this);
  }
}
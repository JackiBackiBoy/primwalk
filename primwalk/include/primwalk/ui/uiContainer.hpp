#ifndef PW_UI_CONTAINER_HEADER
#define PW_UI_CONTAINER_HEADER

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/color.hpp"
#include "primwalk/ui/uiElement.hpp"

// std
#include <memory>
#include <vector>

namespace pw {
  class PW_API UIContainer : public UIElement {
  public:
    UIContainer(glm::vec2 position, int width, int height) :
      UIElement(position), m_Width(width), m_Height(height) {};
    virtual ~UIContainer() {};

    virtual void onRender(UIRenderSystem& renderer) override;
    virtual void handleEvent(const UIEvent& event) override;
    virtual Hitbox hitboxTest(glm::vec2 position) override;

    template<typename UIType, typename Key, typename... Args>
    UIType& makeElement(Key const& key, Args&&...args) {
      auto tmp = std::make_shared<UIType>(std::forward<Args>(args)...);
      tmp->m_Container = this;
      auto& ref = *tmp;

      m_Elements.push_back(std::move(tmp));
      return ref;
    }

    // Getters
    virtual Hitbox getHitbox() override;

    // Setters
    inline void setBackgroundColor(Color color) { m_BackgroundColor = color; }
    inline void setWidth(int width) { m_Width = width; }
    inline void setHeight(int height) { m_Height = height; }
    inline void setBorderRadius(int radius) { m_BorderRadius = radius; }

  private:
    int m_Width;
    int m_Height;
    int m_BorderRadius = 0;
    Color m_BackgroundColor = { 255, 255, 255 };
    std::vector<std::shared_ptr<UIElement>> m_Elements;
  };
}

#endif
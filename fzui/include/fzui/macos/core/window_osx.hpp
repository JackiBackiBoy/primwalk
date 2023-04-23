#ifndef FZ_OSX_WINDOW_HEADER
#define FZ_OSX_WINDOW_HEADER

// FZUI
#include "fzui/core.hpp"
#include "fzui/uiElement.hpp"
#include "fzui/uiContainer.hpp"
#include "fzui/rendering/renderer2d.hpp"

namespace fz {
  class FZ_API WindowOSX : public WindowBase {
    public:
      WindowOSX(const std::string& name, const int& width, const int& height, WindowOSX* parent = nullptr);
      virtual ~WindowOSX();

      int run();

      // Event functions
      virtual void onCreate();
      virtual void onResize() {};
      virtual void onUpdate(float dt) override;
      virtual void onRender(float dt) override;
      virtual void onDestroy() {};

      // UI
      virtual void addElement(UIElement* elem);
      virtual void addContainer(UIContainer* container);

      // Getters
      virtual int getWidth() const override;
      virtual int getHeight() const override;

    private:
      int init();
      void createGraphicsContext();

      // Rendering
      Renderer2D* m_Renderer2D = nullptr;
      Texture m_MinimizeIcon{};
      Texture m_MaximizeIcon{};
      Texture m_CloseIcon{};

      int m_Vsync = 0;
      std::string m_Name = "";
      int m_Width = 0;
      int m_Height = 0;
      std::vector<UIElement*> m_UIElements;
      std::vector<UIContainer*> m_Containers;
      Color m_BackgroundColor = { 20, 20, 20 };
  };
}

#endif

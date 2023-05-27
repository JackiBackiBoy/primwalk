#ifndef FZ_OSX_WINDOW_HEADER
#define FZ_OSX_WINDOW_HEADER

// FZUI
#include "fzui/core.hpp"
#include "fzui/color.hpp"


// std
#include <string>
#include <memory>

namespace fz {
  class FZ_API Renderer;
  class FZ_API UIRenderSystem;

  class FZ_API WindowOSX : public WindowBase {
    public:
      WindowOSX(const std::string& name, const int& width, const int& height, WindowOSX* parent = nullptr);
      virtual ~WindowOSX();

      int run();

      virtual std::vector<std::string> getRequiredVulkanInstanceExtensions() override;
      virtual VkResult createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) override;

      // Event functions
      virtual void onCreate();
      virtual void onResize() {};
      virtual void onUpdate(float dt) override;
      virtual void onRender(float dt) override;
      virtual void onDestroy() {};

      // Getters
      virtual int getWidth() const override;
      virtual int getHeight() const override;

    private:
      int init();
      void createGraphicsContext();

      // Rendering
      std::unique_ptr<Renderer> m_Renderer;
      std::unique_ptr<UIRenderSystem> m_UIRenderSystem;
      id m_Object = nullptr;
      id m_View = nullptr;
      id m_Layer = nullptr;
      std::string m_Name = "";
      int m_Width = 0;
      int m_Height = 0;
      Color m_BackgroundColor = { 20, 20, 20 };
  };

  typedef WindowOSX Window;
}

#endif

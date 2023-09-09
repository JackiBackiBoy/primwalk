#pragma once

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/color.hpp"

// std
#include <string>
#include <memory>

typedef void* id;

namespace pw {
  class PW_API WindowOSX : public WindowBase {
    public:
      WindowOSX(const std::string& name, int width, int height);
      virtual ~WindowOSX();

      int run();

      // Event functions
      virtual void onCreate();
      virtual void onResize() {};
      virtual void onUpdate(float dt) override;
      virtual void onDestroy() {};

      virtual bool shouldClose() override;

    protected:
      int init();

      // Rendering
      id m_Object = nullptr;
      id m_View = nullptr;
      id m_Layer = nullptr;
      bool m_ShouldClose = false;

      friend class GraphicsDevice_Vulkan;
  };

  typedef WindowOSX Window;
}

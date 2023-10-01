#pragma once

// std
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/window.hpp"
#include "primwalk/rendering/graphicsDevice.hpp"
#include "primwalk/rendering/renderer.hpp"
#include "primwalk/rendering/systems/renderSystem3d.hpp"
#include "primwalk/rendering/systems/uiRenderSystem.hpp"
#include "primwalk/ui/GUI.hpp"
#include "primwalk/ui/menuWidget.hpp"
#include "primwalk/ui/subView.hpp"
#include "primwalk/ui/uiElement.hpp"
#include "primwalk/ui/uiEvent.hpp"
#include "primwalk/ui/uiIconButton.hpp"
#include "primwalk/ui/uiImage.hpp"
#include "primwalk/ui/uiLabel.hpp"
#include "primwalk/data/model.hpp"

// std
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <vector>

namespace pw {

  // Singleton design pattern
  class PW_API Application {
    public:
      Application() {};
      virtual ~Application() = default;

      void setWindow(Window* window);
      void run();

      void onRender();

      template<typename UIType, typename Key, typename... Args>
      UIType& makeElement(Key const& key, Args&&...args) {
        auto tmp = std::make_unique<UIType>(std::forward<Args>(args)...);
        auto& ref = *tmp;

        m_UIRenderSystem->submitElement(std::move(tmp));
        return ref;
      }

    private:
      void renderScene(const FrameInfo& frameInfo);

      Window* m_Window = nullptr;
      std::unique_ptr<GraphicsDevice_Vulkan> m_Device;
      std::unique_ptr<Renderer> m_Renderer;
      std::unique_ptr<UIRenderSystem> m_UIRenderSystem;
      std::unique_ptr<RenderSystem3D> m_RenderSystem3D;
      std::vector<std::unique_ptr<SubView>> m_SubViews;
      std::unique_ptr<Model> m_Cube;

      // Multi-threading
      std::atomic<bool> m_Resizing { false };
      std::mutex m_RenderingMutex;
      std::condition_variable m_RenderCondition;

      UIImage engineLogo;
      UILabel testLabel;

      // Top navigation-bar
      UIIconButton fullscreenButton;
      UIIconButton minimizeButton;
      UIIconButton maximizeButton;
      UIIconButton closeButton;

      MenuWidget menu;
      MenuItem fileMenu;
      MenuItem fileSubNewMenu;
      MenuItem fileSubOpenMenu;
      MenuItem editMenu;
      MenuItem assetsMenu;
      MenuItem toolsMenu;
      MenuItem windowMenu;

      MenuItem helpMenu;
      MenuItem helpView;
      MenuItem helpGettingStarted;
      MenuItem helpTipsAndTricks;
      MenuItem helpKeyboardShortcuts;
      MenuItem helpWhatsNew;
      MenuItem helpCheckForUpdates;
      MenuItem helpReleaseNotes;
      MenuItem helpRoadmap;
      MenuItem helpAbout;

      UIIconButton selectButton;
      UIIconButton moveButton;
      UIIconButton rotateButton;
      UIIconButton scaleButton;

      pw::gui::GUI m_GUI;
  };
}


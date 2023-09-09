#pragma once

#ifdef PW_WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

// vendor
#include <vulkan/vulkan.h>

// std
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/color.hpp"
#include "primwalk/ui/subView.hpp"
#include "primwalk/ui/uiElement.hpp"
#include "primwalk/ui/uiEvent.hpp"
#include "primwalk/ui/uiButton.hpp"
#include "primwalk/ui/mouseCursor.hpp"

#include "primwalk/ui/subView.hpp"

namespace pw {
  class PW_API WindowBase {
    public:
      WindowBase(const std::string& name, int width, int height) :
        m_Name(name), m_Width(width), m_Height(height) {};
      virtual ~WindowBase() {}

      //virtual bool isCursorInTitleBar(int x, int y) = 0;
      //virtual bool isCursorOnBorder(int x, int y) = 0;

      // Event functions
      virtual void onUpdate(float dt) {};

      //virtual void processEvent(const UIEvent& event) = 0;

      SubView& makeSubView(int width, int height, glm::vec2 position);

      // Getters
      inline int getWidth() const { return m_Width; }
      inline int getHeight() const { return m_Height; }

      // Setters
      inline void setBackgroundColor(Color color) { m_BackgroundColor = color; }
      //virtual void setMinimumSize(uint32_t width, uint32_t height) = 0;
      //virtual void setCursor(MouseCursor cursor) = 0;

      //virtual void close() = 0;
      virtual bool shouldClose() = 0;

    protected:
      std::string m_Name;
      int m_Width;
      int m_Height;
      uint32_t m_MinWidth = 200;
      uint32_t m_MinHeight = 200;
      Color m_BackgroundColor;
      MouseCursor m_Cursor = MouseCursor::None;
      
      std::vector<std::unique_ptr<SubView> > m_SubViews;
      std::atomic<bool> m_IsMinimized = std::atomic<bool>(false);
      std::mutex m_RenderingMutex;
      std::condition_variable m_RenderingCondition;
      std::atomic<bool> m_CloseFlag = std::atomic<bool>(false);
  };
}

// Platform specific implementation
#if defined(PW_WIN32)
  #include "primwalk/windows/core/window_win32.hpp"
#elif defined(PW_MACOS)
  #include "primwalk/macos/core/window_osx.hpp"
#endif


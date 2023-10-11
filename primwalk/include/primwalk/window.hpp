#pragma once

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/color.hpp"
#include "primwalk/ui/mouseCursor.hpp"
#include "primwalk/ui/uiEvent.hpp"

// std
#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace pw {
  class PW_API WindowBase {
    public:
      WindowBase(const std::string& name, int width, int height) :
        m_Name(name), m_Width(width), m_Height(height) {};
      virtual ~WindowBase() = default;

      virtual bool isCursorInTitleBar(int x, int y) const = 0;
      virtual bool isCursorOnBorder(int x, int y) const = 0;

      virtual void processEvent(const UIEvent& event) = 0;

      inline int getWidth() const { return m_Width; }
      inline int getHeight() const { return m_Height; }

      inline void setBackgroundColor(Color color) { m_BackgroundColor = color; }
      virtual void setCursor(MouseCursor cursor) = 0;
      inline void setResizeCallback(const std::function<void(int width, int height)>& callback) { m_ResizeCallback = callback; }

      virtual void close() = 0;
      virtual bool shouldClose() = 0;

    protected:
      std::string m_Name;
      int m_Width;
      int m_Height;
      Color m_BackgroundColor = Color::White;
      MouseCursor m_Cursor = MouseCursor::None;
      
      std::atomic<bool> m_IsMinimized { false };
      std::atomic<bool> m_CloseFlag { false };
      std::function<void(int, int)> m_ResizeCallback = [](int width, int height) {};
  };
}

// Platform specific implementation
#if defined(PW_WIN32)
  #include "primwalk/windows/core/window_win32.hpp"
#elif defined(PW_MACOS)
  #include "primwalk/macos/core/window_osx.hpp"
#endif


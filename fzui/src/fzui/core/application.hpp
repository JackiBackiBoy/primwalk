#pragma once

// std
#include <string>
#include <unordered_map>

// FZUI
#include "fzui/core/core.hpp"
#include "fzui/core/window.hpp"
#include "fzui/ui/windowInfo.hpp"

namespace fz {

  // Singleton design pattern
  class FZ_API Application {
    public:
      // Remove copy and assignment constructors
      Application(const Application&) = delete;
      Application(Application&&) = delete;
      Application& operator=(const Application&) = delete;
      Application& operator=(Application&&) = delete;

      static Application& Instance();

      Window* addWindow(const std::wstring& title,
          const WindowInfo& info = WindowInfo::DefaultDark);

      void setMainWindow(Window* window);

    private:
      Application() {};
      ~Application() {};

      std::unordered_map<std::wstring, Window*> m_Windows;
      Window* m_MainWindow = nullptr;
  };
}

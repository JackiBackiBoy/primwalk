#pragma once

// std
#include <string>
#include <unordered_map>

// FZUI
#include "fzui/core.hpp"
#include "fzui/window.hpp"

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
      void setMainWindow(WindowWin32* window);

    private:
      Application() {};
      ~Application() {};

      std::unordered_map<std::wstring, WindowWin32*> m_Windows;
      WindowWin32* m_MainWindow = nullptr;
  };
}

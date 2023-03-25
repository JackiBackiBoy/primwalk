#pragma once

// std
#include <string>
#include <unordered_map>

// FZUI
#include "fzui/core.hpp"

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

    private:
      Application() {};
      ~Application() {};
  };
}

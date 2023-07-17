#pragma once

// std
#include <string>
#include <unordered_map>

// primwalk
#include "primwalk/core.hpp"

namespace pw {

  // Singleton design pattern
  class PW_API Application {
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

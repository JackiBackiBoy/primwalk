#pragma once

#define STB_IMAGE_IMPLEMENTATION

// std
#include <string>

// FZUI
#include "fzui/core/core.hpp"

namespace fz {
  class FZ_API Texture {
    public:
      Texture();
      ~Texture() {};

      void loadFromFile(const std::string& path);
      void bind() const;

      // Getters
      unsigned int getID() const;

    private:
      unsigned int m_ID = 0;
  };
}

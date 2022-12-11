#pragma once

#define STB_IMAGE_IMPLEMENTATION

// std
#include <string>

// FZUI
#include "fzui/core/core.hpp"

namespace fz {
  class FZ_API Texture {
    public:
      Texture(const std::string& path);
      ~Texture() {};

      void bind() const;

    private:
      unsigned int m_ID;
  };
}

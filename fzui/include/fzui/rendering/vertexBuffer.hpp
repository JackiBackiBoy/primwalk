#pragma once

// std
#include <vector>

// FZUI
#include "fzui/core.hpp"
#include "fzui/vertex.hpp"

// vendor
#include <glad/glad.h>

namespace fz {
  class FZ_API VertexBuffer {
    public:
      VertexBuffer(const int& usage);
      ~VertexBuffer();

      void bind();
      void unbind();
      void create(int size);

    private:
      unsigned int m_ID = 0;
      int m_Usage = 0;

      friend class VertexArray;
  };
}

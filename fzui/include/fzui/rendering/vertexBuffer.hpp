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
      VertexBuffer(const std::vector<Vertex>& vertices, const int& usage);
      ~VertexBuffer();

      void bind();
      void unbind();
      void create();

    private:
      unsigned int m_ID = 0;
      std::vector<Vertex> m_Vertices;
      int m_Usage = 0;
  };
}

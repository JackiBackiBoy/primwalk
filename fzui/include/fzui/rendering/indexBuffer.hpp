#pragma once

// std
#include <vector>

// FZUI
#include "fzui/core.hpp"

// vendor
#include <glad/glad.h>

namespace fz {
  class FZ_API IndexBuffer {
    public:
      IndexBuffer(const std::vector<unsigned int>& indices, const int& usage);
      ~IndexBuffer();

      void bind();
      void unbind();
      void create(int size);

    private:
      unsigned int m_ID = 0;
      std::vector<unsigned int> m_Indices;
      int m_Usage = GL_DYNAMIC_DRAW;

      friend class VertexArray;
  };
}
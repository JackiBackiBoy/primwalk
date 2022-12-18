#pragma once

// FZUI
#include "fzui/core/core.hpp"
#include "fzui/rendering/bufferLayout.hpp"
#include "fzui/rendering/vertexBuffer.hpp"
#include "fzui/rendering/indexBuffer.hpp"

namespace fz {
  class FZ_API VertexArray {
    public:
      VertexArray(const BufferLayout& layout, const VertexBuffer& vbo,
          const IndexBuffer& ebo);
      ~VertexArray() {};

      void bind();
      void unbind();
      void create();

    private:
      unsigned int m_ID = 0;
      BufferLayout m_BufferLayout;
      VertexBuffer m_VBO;
      IndexBuffer m_EBO;
  };
}

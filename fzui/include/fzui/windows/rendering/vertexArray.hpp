#pragma once

// FZUI
#include "fzui/core.hpp"
#include "fzui/windows/rendering/bufferLayout.hpp"
#include "fzui/windows/rendering/vertexBuffer.hpp"
#include "fzui/windows/rendering/indexBuffer.hpp"

namespace fz {
  class FZ_API VertexArray {
    public:
      VertexArray(BufferLayout& layout, VertexBuffer* vbo, IndexBuffer* ibo);
      ~VertexArray();

      void bind();
      void unbind();
      void create();

    private:
      unsigned int m_ID = 0;
      BufferLayout m_BufferLayout;
      VertexBuffer* m_VBO = nullptr;
      IndexBuffer* m_IBO = nullptr;
  };
}

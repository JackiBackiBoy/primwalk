#pragma once

// FZUI
#include "fzui/core.hpp"
#include "fzui/rendering/bufferLayout.hpp"
#include "fzui/rendering/vertexBuffer.hpp"
#include "fzui/rendering/indexBuffer.hpp"

namespace fz {
  class FZ_API VertexArray {
    public:
      VertexArray(BufferLayout layout, VertexBuffer* vbo, IndexBuffer* ibo);
      ~VertexArray();

      void bind();
      void unbind();
      void create();

      // Getters
      unsigned int getVertexSize() const;

    private:
      unsigned int m_ID = 0;
      BufferLayout m_BufferLayout{};
      VertexBuffer* m_VBO = nullptr;
      IndexBuffer* m_IBO = nullptr;
  };
}

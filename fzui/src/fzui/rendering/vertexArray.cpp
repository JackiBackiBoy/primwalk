// FZUI
#include "fzui/rendering/vertexArray.hpp"

namespace fz {
  VertexArray::VertexArray(const BufferLayout& layout, const VertexBuffer& vbo,
      const IndexBuffer& ibo) :
    m_BufferLayout(layout), m_VBO(vbo), m_IBO(ibo) {
      //m_VBO.unbind();
      //unbind();
  };

  void VertexArray::bind() {
    glBindVertexArray(m_ID);
  }

  void VertexArray::unbind() {
    glBindVertexArray(0);
  }

  void VertexArray::create() {
    // Create VAO
    glCreateVertexArrays(1, &m_ID);
    bind();

    // Create VBO and IBO
    m_VBO.create();
    m_IBO.create();

    std::vector<BufferAttribute> attributes = m_BufferLayout.getAttributes();
    int offset = 0;

    for (size_t i = 0; i < attributes.size(); i++) {
      glEnableVertexAttribArray(i);
      glVertexAttribPointer(i, attributes[i].count, attributes[i].glType,
          GL_FALSE, m_BufferLayout.getStride(),
          reinterpret_cast<void*>(offset));

      offset += attributes[i].byteSize;
    }
  }
}

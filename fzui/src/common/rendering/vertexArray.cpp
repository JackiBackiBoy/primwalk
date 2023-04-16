// FZUI
#include "fzui/rendering/vertexArray.hpp"

namespace fz {
  VertexArray::VertexArray(BufferLayout layout, VertexBuffer* vbo,
      IndexBuffer* ibo) :
    m_BufferLayout(layout), m_VBO(vbo), m_IBO(ibo) {
      //m_VBO.unbind();
      //unbind();
  };

  VertexArray::~VertexArray() {
    glDeleteVertexArrays(1, &m_ID);
  }

  void VertexArray::bind() {
    glBindVertexArray(m_ID);
  }

  void VertexArray::unbind() {
    glBindVertexArray(0);
  }

  void VertexArray::create() {
    // Create VAO
    glGenVertexArrays(1, &m_ID);

    bind();

    // Create VBO and IBO
    m_VBO->create(0);
    m_IBO->create(0);

    std::vector<BufferAttribute> attributes = m_BufferLayout.getAttributes();
    int offset = 0;

    for (size_t i = 0; i < attributes.size(); i++) {
      glEnableVertexAttribArray(i);
      glVertexAttribPointer(i, attributes[i].count, attributes[i].glType,
          GL_FALSE, m_BufferLayout.getStride(),
          reinterpret_cast<void*>(offset));

      offset += attributes[i].byteSize;
    }

    m_VBO->unbind();
    unbind();
  }

  // Getters
  unsigned int VertexArray::getVertexSize() const {
    unsigned int size = 0;

    for (auto& a : m_BufferLayout.getAttributes()) {
      size += a.byteSize;
    }

    return size;
  }
}

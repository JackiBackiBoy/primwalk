// FZUI
#include "fzui/windows/rendering/indexBuffer.hpp"

namespace fz {
  IndexBuffer::IndexBuffer(const std::vector<unsigned int>& indices, const int& usage) :
    m_Indices(indices), m_Usage(usage) {
  }

  IndexBuffer::~IndexBuffer() {
    glDeleteBuffers(1, &m_ID);
  }

  void IndexBuffer::bind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
  }

  void IndexBuffer::unbind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  void IndexBuffer::create() {
    glGenBuffers(1, &m_ID);
    bind();
    // TODO: Fix dynamic storage data
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 1024, nullptr, m_Usage);
  }
}

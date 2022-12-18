// FZUI
#include "fzui/rendering/indexBuffer.hpp"

namespace fz {
  IndexBuffer::IndexBuffer(const std::vector<unsigned int>& indices, const int& usage) :
    m_Indices(indices), m_Usage(usage) {
  }

  void IndexBuffer::bind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
  }

  void IndexBuffer::unbind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  void IndexBuffer::create() {
    glCreateBuffers(1, &m_ID);
    bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * m_Indices.size(), m_Indices.data(), m_Usage);
  }
}
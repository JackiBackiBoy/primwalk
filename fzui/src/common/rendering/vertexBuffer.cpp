// FZUI
#include "fzui/rendering/vertexBuffer.hpp"

namespace fz {
  VertexBuffer::VertexBuffer(const std::vector<Vertex>& vertices, const int& usage) :
    m_Vertices(vertices), m_Usage(usage) {
  }

  VertexBuffer::~VertexBuffer() {
    glDeleteBuffers(1, &m_ID);
  }

  void VertexBuffer::bind() {
    glBindBuffer(GL_ARRAY_BUFFER, m_ID);
  }

  void VertexBuffer::unbind() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  void VertexBuffer::create() {
    glGenBuffers(1, &m_ID);
    bind();
    // TODO: Define max number of vertices (512 for now) to be dynamic
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 512, nullptr, m_Usage);
  }
}

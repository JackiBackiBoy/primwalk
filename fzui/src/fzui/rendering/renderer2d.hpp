#pragma once

// std
#include <vector>
#include <memory>

// FZUI
#include "fzui/core/core.hpp"
#include "fzui/rendering/vertexBuffer.hpp"
#include "fzui/rendering/indexBuffer.hpp"
#include "fzui/rendering/vertexArray.hpp"

namespace fz {
  class FZ_API Renderer2D {
    public:
      Renderer2D();
      ~Renderer2D() {};

      void begin();
      void end();
      void render();

      // Draw functions
      void drawQuad(const glm::vec2& a, const glm::vec2& b,
                    const glm::vec2& c, const glm::vec2& d);

    private:
      std::vector<Vertex> m_Vertices;
      std::vector<unsigned int> m_Indices;
      std::unique_ptr<VertexBuffer> m_VBO;
      std::unique_ptr<IndexBuffer> m_IBO;
      std::unique_ptr<VertexArray> m_VAO;
  };
}

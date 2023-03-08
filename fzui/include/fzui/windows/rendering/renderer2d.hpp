#pragma once

// std
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

// FZUI
#include "fzui/core.hpp"
#include "fzui/windows/rendering/vertexBuffer.hpp"
#include "fzui/windows/rendering/indexBuffer.hpp"
#include "fzui/windows/rendering/vertexArray.hpp"
#include "fzui/windows/data/fonts/fontCharacter.hpp"
#include "fzui/windows/data/fonts/fontFace.hpp"
#include "fzui/windows/data/texture.hpp"
#include "fzui/windows/color.hpp"

namespace fz {
  class FZ_API Renderer2D {
    public:
      Renderer2D();
      ~Renderer2D() {};

      void begin();
      void end();
      void render();

      // Draw functions
      void drawRect(const int& width, const int& height,
                    const glm::vec2& pos, const Color& color,
                    const unsigned int& texID);
      void drawQuad(const glm::vec2& a, const glm::vec2& b,
                    const glm::vec2& c, const glm::vec2& d,
                    const Color& color, const unsigned int& texID);
      void drawText(const std::string& text, const glm::vec2& pos,
                    const float& fontSize, const Color& color);

    private:
      std::vector<Vertex> m_Vertices;
      std::vector<unsigned int> m_Indices;
      std::unique_ptr<VertexBuffer> m_VBO;
      std::unique_ptr<IndexBuffer> m_IBO;
      std::unique_ptr<VertexArray> m_VAO;
      FontFace* m_Font = nullptr;
  };
}
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
#include "fzui/windows/data/shader.hpp"

namespace fz {
  class FZ_API Renderer2D {
    public:
      Renderer2D();
      ~Renderer2D();

      void begin();
      void end();

      // Setters
      void setViewport(const unsigned int& width,
                       const unsigned int& height);

      // Draw functions
      void drawRect(const int& width, const int& height,
                    const glm::vec2& pos, const Color& color,
                    Texture* texture = nullptr);
                    void drawRect(const int& width, const int& height,
                    const glm::vec2& pos, const Color& color,
                    unsigned int texture);
      void drawQuad(const glm::vec2& a, const glm::vec2& b,
                    const glm::vec2& c, const glm::vec2& d,
                    const Color& color, Texture* texture = nullptr);
      void drawQuad(const glm::vec2& a, const glm::vec2& b,
                    const glm::vec2& c, const glm::vec2& d,
                    const Color& color, unsigned int texture);
      void drawText(const std::string& text, const glm::vec2& pos,
                    const float& fontSize, const Color& color);

    private:
      std::vector<Vertex> m_Vertices;
      std::vector<unsigned int> m_Indices;
      std::vector<Vertex> m_TextVertices;
      std::vector<unsigned int> m_TextIndices;

      VertexBuffer* m_VBO = nullptr;
      IndexBuffer* m_IBO = nullptr;
      VertexArray* m_VAO = nullptr;
      VertexBuffer* m_TextVBO = nullptr;
      IndexBuffer* m_TextIBO = nullptr;
      VertexArray* m_TextVAO = nullptr;

      std::unordered_map<unsigned int, unsigned int> m_TextureUnits;
      unsigned int m_NumTextures = 0;
      glm::vec2 m_Viewport = { 0, 0 };

      Shader shader{};
      Shader glyphShader{};
      FontFace* m_Font = nullptr;
  };
}

#pragma once

// std
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

// FZUI
#include "fzui/core.hpp"
#include "fzui/rendering/vertexBuffer.hpp"
#include "fzui/rendering/indexBuffer.hpp"
#include "fzui/rendering/vertexArray.hpp"
#include "fzui/data/fonts/fontCharacter.hpp"
#include "fzui/data/fonts/fontFace.hpp"
#include "fzui/data/texture.hpp"
#include "fzui/color.hpp"
#include "fzui/data/shader.hpp"

namespace fz {
  class FZ_API Renderer2D {
    public:
      Renderer2D();
      ~Renderer2D();

      void begin();
      void end();

      // Setters
      void setViewport(unsigned int width,
                       unsigned int height);

      // Draw functions
      void drawRect(int width, int height,
                    const glm::vec2& pos, const Color& color,
                    int borderRadius = 0, Texture* texture = nullptr);
      void drawQuad(const glm::vec2& a, const glm::vec2& b,
                    const glm::vec2& c, const glm::vec2& d,
                    const Color& color, Texture* texture = nullptr);
      void drawText(const std::string& text, const glm::vec2& pos,
                    float fontSize, const Color& color,
                    FontFace* fontFace = nullptr);

    private:
      unsigned int VBO, VAO, EBO;
      unsigned int textVBO, textVAO, textEBO;
      std::vector<RectVertex> m_Vertices;
      std::vector<unsigned int> m_Indices;
      std::vector<TextVertex> m_TextVertices;
      std::vector<unsigned int> m_TextIndices;

      std::unordered_map<unsigned int, unsigned int> m_TextureUnits;
      std::unordered_map<unsigned int, unsigned int> m_FontTextureUnits;
      unsigned int m_NumTextures = 0;
      unsigned int m_NumFontTextures = 0;
      glm::vec2 m_Viewport = { 0, 0 };

      Shader shader{};
      Shader glyphShader{};
      FontFace* m_SmallFont = nullptr;
      FontFace* m_MediumFont = nullptr;
  };
}

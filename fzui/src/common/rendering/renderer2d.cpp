// FZUI
#include "fzui/rendering/renderer2d.hpp"

// std
#include <algorithm>
#include <array>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>

// vendor
#include <glad/glad.h>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace fz {
  static Texture whiteTex;

  // TODO: Make texture atlas initialization dynamic
  Renderer2D::Renderer2D() {
    unsigned char whitePixel[4] = { 255, 255, 255, 255 };

    whiteTex.create(1, 1, whitePixel); // default texture

    m_Vertices = std::vector<RectVertex>();
    m_Indices = std::vector<unsigned int>();
    m_TextVertices = std::vector<TextVertex>();
    m_TextIndices = std::vector<unsigned int>();
    m_TextureUnits = std::unordered_map<unsigned int, unsigned int>();
    m_FontTextureUnits = std::unordered_map<unsigned int, unsigned int>();
    m_NumTextures = 0;
    m_NumFontTextures = 0;

    // Rectangle VAO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(RectVertex) * 2000, nullptr, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 10000, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(RectVertex), (void*)offsetof(RectVertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(RectVertex), (void*)offsetof(RectVertex, texCoord));
    
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(RectVertex), (void*)offsetof(RectVertex, texIndex));
    
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(RectVertex), (void*)offsetof(RectVertex, color));
    
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(RectVertex), (void*)offsetof(RectVertex, borderRadius));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Text VAO
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glGenBuffers(1, &textEBO);
    glBindVertexArray(textVAO);

    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TextVertex) * 2000, nullptr, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, textEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 10000, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void*)offsetof(TextVertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void*)offsetof(TextVertex, texCoord));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void*)offsetof(TextVertex, texIndex));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void*)offsetof(TextVertex, color));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    // Enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Shaders
    shader.loadShader(ShaderType::Vertex, "assets/shaders/uiShader.vert");
    shader.loadShader(ShaderType::Fragment, "assets/shaders/uiShader.frag");
    shader.compileShaders();
    shader.bind();
    {
    auto loc = glGetUniformLocation(shader.getID(), "u_Textures");
    int samplers[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    glUniform1iv(loc, 16, samplers);
    }

    glUseProgram(0);

    // Glyph shader
    glyphShader.loadShader(ShaderType::Vertex, "assets/shaders/glyphShader.vert");
    glyphShader.loadShader(ShaderType::Fragment, "assets/shaders/glyphShader.frag");
    glyphShader.compileShaders();
    glyphShader.bind();
    {
    auto loc = glGetUniformLocation(glyphShader.getID(), "u_Textures");
    int samplers[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    glUniform1iv(loc, 16, samplers);
    }

    // glm::mat4 projMat = glm::ortho(0.0f, (float)m_Width, (float)m_Height, 0.0f);
    // shader.setUniformMat4("projMat", projMat);
    m_SmallFont = FontFace::create("assets/fonts/segoeui.ttf", 12.0);
    m_MediumFont = FontFace::create("assets/fonts/segoeui.ttf", 32.0);
  }

  Renderer2D::~Renderer2D() {
  }

  void Renderer2D::begin() {
    
  }

  void Renderer2D::end() {
    float c = 0.1f;
    glClearColor(c, c, c, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glm::mat4 projMat = glm::ortho(0.0f, (float)m_Viewport.x, (float)m_Viewport.y, 0.0f);
    shader.bind();
    shader.setUniformMat4("projMat", projMat);
    shader.setUniformVec2("u_Resolution", m_Viewport);

    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, whiteTex.getID());

    for (auto t : m_TextureUnits) {
      unsigned int index = t.second;
      unsigned int texID = t.first;
      glActiveTexture(GL_TEXTURE0 + index);
      glBindTexture(GL_TEXTURE_2D, texID);
    }

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(RectVertex) * m_Vertices.size(), m_Vertices.data());
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(unsigned int) * m_Indices.size(), m_Indices.data());

    // Render the batched data
    glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_INT, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // // ------ Text rendering ------
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);

    glyphShader.bind();
    glyphShader.setUniformMat4("projMat", projMat);

    for (auto t : m_FontTextureUnits) {
      unsigned int index = t.second;
      unsigned int texID = t.first;
      glActiveTexture(GL_TEXTURE0 + index);
      glBindTexture(GL_TEXTURE_2D, texID);
    }

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(TextVertex) * m_TextVertices.size(), m_TextVertices.data());
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(unsigned int) * m_TextIndices.size(), m_TextIndices.data());

    // Render the batched data
    glDrawElements(GL_TRIANGLES, m_TextIndices.size(), GL_UNSIGNED_INT, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Cleanup
    m_Vertices.clear();
    m_Indices.clear();
    m_TextVertices.clear();
    m_TextIndices.clear();
    m_NumTextures = 0;
    m_NumFontTextures = 0;
    m_TextureUnits.clear();
    //m_FontTextureUnits.clear();
  }

  void Renderer2D::setViewport(unsigned int width,
                               unsigned int height) {
    m_Viewport = { width, height };
    //glViewport(0, 0, width, height);
  }

  // Draw functions
  void Renderer2D::drawRect(int width, int height,
                            const glm::vec2& pos, const Color& color,
                            int borderRadius, Texture* texture) {
    // Retrieve the corresponding texture index value
    float texIndex = 0.0f; // default tex value

    if (texture != nullptr) {
      auto res = m_TextureUnits.find(texture->getID());

      if (res == m_TextureUnits.end()) { // texture not bound
        m_NumTextures++;
        m_TextureUnits.insert({ texture->getID(), m_NumTextures });
        texIndex = (float)m_NumTextures;
      }
      else {
        texIndex = (float)res->second;
      }
    }

    // TODO: Use memcpy instead on predefined array size
    m_Vertices.emplace_back(RectVertex { { pos.x + width, pos.y, 0.0f }, { 1.0f, 1.0f }, texIndex, Color::normalize(color), (float)borderRadius });
    m_Vertices.emplace_back(RectVertex { { pos.x, pos.y, 0.0f }, { 0.0f, 1.0f }, texIndex, Color::normalize(color), (float)borderRadius });
    m_Vertices.emplace_back(RectVertex { { pos.x, pos.y + height, 0.0f }, { 0.0f, 0.0f }, texIndex, Color::normalize(color), (float)borderRadius });
    m_Vertices.emplace_back(RectVertex { { pos.x + width, pos.y + height, 0.0f }, { 1.0f, 0.0f }, texIndex, Color::normalize(color), (float)borderRadius });

    // TODO: Dynamic indices
    unsigned int indexOffset = (m_Indices.size() / 6) * 4;
    m_Indices.emplace_back(0 + indexOffset);
    m_Indices.emplace_back(1 + indexOffset);
    m_Indices.emplace_back(2 + indexOffset);
    m_Indices.emplace_back(0 + indexOffset);
    m_Indices.emplace_back(2 + indexOffset);
    m_Indices.emplace_back(3 + indexOffset);
  }

  void Renderer2D::drawQuad(const glm::vec2& a, const glm::vec2& b,
                            const glm::vec2& c, const glm::vec2& d,
                            const Color& color, Texture* texture) {
    std::array<glm::vec2, 4> points = { a, b, c, d };

    // 1. Estimate center point as the mean of the points
    glm::vec2 center((a.x + b.x + c.x + d.x) / 4.0f,
                     (a.y + b.y + c.y + d.y) / 4.0f);

    std::sort(points.begin(), points.end(), [center](glm::vec2 u, glm::vec2 v) {
      glm::vec2 uDir = u - center;
      glm::vec2 vDir = v - center;
      uDir.y = -uDir.y; // flip sign due to screen space coordinates
      vDir.y = -vDir.y; // flip sign due to screen space coordinates

      // Compute angle between center point and point u
      float uAngle = std::atan2(uDir.y, uDir.x);
      uAngle += uAngle < 0.0f ? glm::two_pi<float>() : 0.0f;

      // Compute angle between center point and point v
      float vAngle = std::atan2(vDir.y, vDir.x);
      vAngle += vAngle < 0.0f ? glm::two_pi<float>() : 0.0f;

      if (uAngle < vAngle) {
        return true;
      }

      // If uAngle is not less than vAngle, we must then sort by lowest length
      if (uAngle == vAngle && glm::length(uDir) < glm::length(vDir)) {
        return true;
      }

      return false;
    });

    // Retrieve the corresponding texture index value
    float texIndex = 0.0f; // default tex value

    if (texture != nullptr) {
      auto res = m_TextureUnits.find(texture->getID());

      if (res == m_TextureUnits.end()) { // texture not bound
        m_NumTextures++;
        m_TextureUnits.insert({ texture->getID(), m_NumTextures });
        texIndex = (float)m_NumTextures;
      }
      else {
        texIndex = (float)res->second;
      }
    }

    // TODO: Use memcpy instead on predefined array size
    m_Vertices.emplace_back(RectVertex { { points[0].x, points[0].y, 0.0f }, { 1.0f, 1.0f }, texIndex, Color::normalize(color) });
    m_Vertices.emplace_back(RectVertex { { points[1].x, points[1].y, 0.0f }, { 0.0f, 1.0f }, texIndex, Color::normalize(color) });
    m_Vertices.emplace_back(RectVertex { { points[2].x, points[2].y, 0.0f }, { 0.0f, 0.0f }, texIndex, Color::normalize(color) });
    m_Vertices.emplace_back(RectVertex { { points[3].x, points[3].y, 0.0f }, { 1.0f, 0.0f }, texIndex, Color::normalize(color) });

    // TODO: Dynamic indices
    unsigned int indexOffset = (m_Indices.size() / 6) * 4;
    m_Indices.emplace_back(0 + indexOffset);
    m_Indices.emplace_back(1 + indexOffset);
    m_Indices.emplace_back(2 + indexOffset);
    m_Indices.emplace_back(0 + indexOffset);
    m_Indices.emplace_back(2 + indexOffset);
    m_Indices.emplace_back(3 + indexOffset);
  }

  void Renderer2D::drawText(const std::string& text, const glm::vec2& pos,
                            float fontSize, const Color& color,
                            FontFace* fontFace) {
    FontFace* font = fontFace;
    float texIndex = 0.0f;

    if (font == nullptr) {
      font = m_SmallFont;
      texIndex = 0.0f;
    }
    
    auto res = m_FontTextureUnits.find(font->getTextureID());
    if (res == m_FontTextureUnits.end()) { // texture atlas not found
      m_FontTextureUnits.insert({ font->getTextureID(), m_NumFontTextures });
      texIndex = static_cast<float>(m_NumFontTextures);
      m_NumFontTextures++;
      std::cout << texIndex << std::endl;
    }
    else {
      
      texIndex = static_cast<float>(res->second);
    }

    float posX = pos.x;
    float scaling = fontSize / font->getFontSize();
    float offsetX = 0.0f;

    for (size_t i = 0; i < text.length(); i++) {
      GlyphData glyph = font->getGlyph(static_cast<uint32_t>(text[i]));

      if (i == 0) {
        offsetX = 0.0f;
      }

      if (text[i] != ' ') {
        // TODO: Use memcpy instead on predefined array size
        m_TextVertices.emplace_back(TextVertex { { posX + (glyph.bearingX + glyph.width) * scaling + offsetX, pos.y + (font->getMaxHeight()- glyph.bearingY) * scaling,                  0.0f }, { glyph.texRightX, glyph.texTopY }, texIndex, Color::normalize(color) });
        m_TextVertices.emplace_back(TextVertex { { posX + (glyph.bearingX) * scaling + offsetX, pos.y + (font->getMaxHeight() - glyph.bearingY) * scaling,                               0.0f }, { glyph.texLeftX, glyph.texTopY }, texIndex, Color::normalize(color) });
        m_TextVertices.emplace_back(TextVertex { { posX + (glyph.bearingX) * scaling + offsetX, pos.y + (font->getMaxHeight()- glyph.bearingY + glyph.height) * scaling,                 0.0f }, { glyph.texLeftX, glyph.texBottomY }, texIndex, Color::normalize(color) });
        m_TextVertices.emplace_back(TextVertex { { posX + (glyph.bearingX + glyph.width) * scaling + offsetX,  pos.y + (font->getMaxHeight() - glyph.bearingY + glyph.height) * scaling, 0.0f }, { glyph.texRightX, glyph.texBottomY }, texIndex, Color::normalize(color) });

        // TODO: Dynamic indices
        unsigned int indexOffset = (m_TextIndices.size() / 6) * 4;
        m_TextIndices.emplace_back(0 + indexOffset);
        m_TextIndices.emplace_back(1 + indexOffset);
        m_TextIndices.emplace_back(2 + indexOffset);
        m_TextIndices.emplace_back(0 + indexOffset);
        m_TextIndices.emplace_back(2 + indexOffset);
        m_TextIndices.emplace_back(3 + indexOffset);
      }

      posX += glyph.advanceX * scaling;
    }
  }
}

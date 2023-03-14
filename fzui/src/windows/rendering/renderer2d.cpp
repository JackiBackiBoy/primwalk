// FZUI
#include "fzui/windows/rendering/renderer2d.hpp"
#include "fzui/windows/data/fonts/fontManager.hpp"

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

    m_Vertices = std::vector<Vertex>();
    m_Indices = std::vector<unsigned int>();
    m_TextVertices = std::vector<Vertex>();
    m_TextIndices = std::vector<unsigned int>();


    m_TextureUnits = std::unordered_map<unsigned int, unsigned int>();
    m_NumTextures = 0;

    BufferLayout bufferLayout;
    bufferLayout.addAttribute<float>("Position", 3);
    bufferLayout.addAttribute<float>("TexCoord", 2);
    bufferLayout.addAttribute<float>("TexIndex", 1);
    bufferLayout.addAttribute<float>("Color", 3);

    m_VBO = new VertexBuffer(m_Vertices, GL_DYNAMIC_DRAW);
    m_IBO = new IndexBuffer(m_Indices, GL_DYNAMIC_DRAW);
    m_VAO = new VertexArray(bufferLayout, m_VBO, m_IBO);
    m_VAO->create();

    m_TextVBO = new VertexBuffer(m_TextVertices, GL_DYNAMIC_DRAW);
    m_TextIBO = new IndexBuffer(m_TextIndices, GL_DYNAMIC_DRAW);
    m_TextVAO = new VertexArray(bufferLayout, m_TextVBO, m_TextIBO);
    m_TextVAO->create();
    

    // ------ FreeType ------
    
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
    m_MediumFont = FontFace::create("assets/fonts/segoeui.ttf", 24.0);

    FontManager& instance = FontManager::Instance();
    instance.setDefaultSmallFont(m_SmallFont);
    instance.setDefaultMediumFont(m_MediumFont);
  }

  Renderer2D::~Renderer2D() {
    delete m_VAO;
    delete m_VBO;
    delete m_IBO;
  }

  void Renderer2D::begin() {
    
  }

  void Renderer2D::end() {
    float c = 0.1f;
    glClearColor(c, c, c, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    //m_VAO->bind();
    m_VAO->bind();
    m_VBO->bind();

    glm::mat4 projMat = glm::ortho(0.0f, (float)m_Viewport.x, (float)m_Viewport.y, 0.0f);
    shader.bind();
    shader.setUniformMat4("projMat", projMat);

    glBindTextureUnit(0, whiteTex.getID());

    for (auto t : m_TextureUnits) {
      unsigned int index = t.second;
      unsigned int texID = t.first;
      glBindTextureUnit(index, texID);
    }

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * m_Vertices.size(), m_Vertices.data());
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(unsigned int) * m_Indices.size(), m_Indices.data());

    // Render the batched data
    glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_INT, (void*)0);

    m_VBO->unbind();
    m_VAO->unbind();

    // // ------ Text rendering ------
    m_TextVAO->bind();
    m_TextVBO->bind();

    glyphShader.bind();
    glyphShader.setUniformMat4("projMat", projMat);
    glBindTextureUnit(0, m_SmallFont->m_TextureAtlas);
    glBindTextureUnit(1, m_MediumFont->m_TextureAtlas);

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * m_TextVertices.size(), m_TextVertices.data());
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(unsigned int) * m_TextIndices.size(), m_TextIndices.data());

    // Render the batched data
    glDrawElements(GL_TRIANGLES, m_TextIndices.size(), GL_UNSIGNED_INT, (void*)0);

    m_TextVBO->unbind();
    m_TextVAO->unbind();

    // Cleanup
    m_Vertices.clear();
    m_Indices.clear();
    m_TextureUnits.clear();
    m_NumTextures = 0;

    m_TextVertices.clear();
    m_TextIndices.clear();
  }

  void Renderer2D::setViewport(const unsigned int& width,
                               const unsigned int& height) {
    m_Viewport = { width, height };
    glViewport(0, 0, width, height);
  }

  // Draw functions
  void Renderer2D::drawRect(const int& width, const int& height,
                            const glm::vec2& pos, const Color& color,
                            Texture* texture) {
    drawQuad({ pos.x, pos.y }, { pos.x, pos.y + height },
             { pos.x + width, pos.y + height }, { pos.x + width, pos.y },
             color, texture);
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
    m_Vertices.push_back({ { points[0].x, points[0].y, 0.0f }, { 1.0f, 1.0f }, texIndex, Color::normalize(color) });
    m_Vertices.push_back({ { points[1].x, points[1].y, 0.0f }, { 0.0f, 1.0f }, texIndex, Color::normalize(color) });
    m_Vertices.push_back({ { points[2].x, points[2].y, 0.0f }, { 0.0f, 0.0f }, texIndex, Color::normalize(color) });
    m_Vertices.push_back({ { points[3].x, points[3].y, 0.0f }, { 1.0f, 0.0f }, texIndex, Color::normalize(color) });

    // TODO: Dynamic indices
    unsigned int indexOffset = (m_Indices.size() / 6) * 4;
    m_Indices.push_back(0 + indexOffset);
    m_Indices.push_back(1 + indexOffset);
    m_Indices.push_back(2 + indexOffset);
    m_Indices.push_back(0 + indexOffset);
    m_Indices.push_back(2 + indexOffset);
    m_Indices.push_back(3 + indexOffset);
  }

  void Renderer2D::drawText(const std::string& text, const glm::vec2& pos,
                            const float& fontSize, const Color& color,
                            FontFace* fontFace) {
    FontFace* font = m_SmallFont;
    float texIndex = 0.0f;

    if (fontSize >= 26.0) {
      font = m_MediumFont;
      texIndex = 1.0f;
    }

    if (fontFace != nullptr) {
      font = fontFace;
    }

    float posX = pos.x;
    float scaling = fontSize / font->getFontSize();

    for (char c : text) {
      GlyphData glyph = font->getGlyph((uint32_t)c);

      float offset = 0.0f;

      if (c != ' ') {
        // TODO: Use memcpy instead on predefined array size
        m_TextVertices.push_back({ { posX + (glyph.bearingX + glyph.width) * scaling, pos.y + (font->getMaxHeight()- glyph.bearingY) * scaling + offset,                 0.0f }, { glyph.texRightX, glyph.texTopY }, texIndex, Color::normalize(color) });
        m_TextVertices.push_back({ { posX + (glyph.bearingX) * scaling, pos.y + (font->getMaxHeight() - glyph.bearingY) * scaling + offset,                               0.0f }, { glyph.texLeftX, glyph.texTopY }, texIndex, Color::normalize(color) });
        m_TextVertices.push_back({ { posX + (glyph.bearingX) * scaling, pos.y + (font->getMaxHeight()- glyph.bearingY + glyph.height) * scaling + offset,                0.0f }, { glyph.texLeftX, glyph.texBottomY }, texIndex, Color::normalize(color) });
        m_TextVertices.push_back({ { posX + (glyph.bearingX + glyph.width) * scaling,  pos.y + (font->getMaxHeight() - glyph.bearingY + glyph.height) * scaling + offset, 0.0f }, { glyph.texRightX, glyph.texBottomY }, texIndex, Color::normalize(color) });

        // TODO: Dynamic indices
        unsigned int indexOffset = (m_TextIndices.size() / 6) * 4;
        m_TextIndices.push_back(0 + indexOffset);
        m_TextIndices.push_back(1 + indexOffset);
        m_TextIndices.push_back(2 + indexOffset);
        m_TextIndices.push_back(0 + indexOffset);
        m_TextIndices.push_back(2 + indexOffset);
        m_TextIndices.push_back(3 + indexOffset);
      }

      posX += glyph.advanceX * scaling;
    }
  }
}

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

namespace fz {
  static Texture tex0;
  static Texture tex1;

  // TODO: Make texture atlas initialization dynamic
  Renderer2D::Renderer2D() {
    unsigned char whitePixel[4] = { 255, 255, 255, 255 };

    tex0.create(1, 1, whitePixel);
    tex1.loadFromFile("assets/textures/fzcoach16x16.png");

    // Load texture atlas
    m_Font = FontFace::create("assets/fonts/segoeuismall.fnt");

    m_Vertices = std::vector<Vertex>();
    m_Indices = std::vector<unsigned int>();

    BufferLayout bufferLayout;
    bufferLayout.addAttribute<float>("Position", 3);
    bufferLayout.addAttribute<float>("TexCoord", 2);
    bufferLayout.addAttribute<float>("TexIndex", 1);
    bufferLayout.addAttribute<float>("Color", 3);

    m_VBO = std::make_unique<VertexBuffer>(VertexBuffer(m_Vertices, GL_DYNAMIC_DRAW));
    m_IBO = std::make_unique<IndexBuffer>(IndexBuffer(m_Indices, GL_DYNAMIC_DRAW));
    m_VAO = std::make_unique<VertexArray>(VertexArray(bufferLayout, *m_VBO, *m_IBO));
    m_VAO->create();


    // Enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

  void Renderer2D::begin() {

  }

  void Renderer2D::end() {

    float c = 0.1f;
    glClearColor(c, c, c, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    m_VAO->bind();
    glBindTextureUnit(0, tex0.getID());
    glBindTextureUnit(1, tex1.getID());
    glBindTextureUnit(2, m_Font->m_TextureAtlas->getID());

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * m_Vertices.size(), m_Vertices.data());
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(unsigned int) * m_Indices.size(), m_Indices.data());

    // Renders the batched data
    glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_INT, (void*)0);
    m_VAO->unbind();

    m_Vertices.clear();
    m_Indices.clear();
  }

  // Draw functions
  void Renderer2D::drawRect(const int& width, const int& height,
                            const glm::vec2& pos, const glm::vec3& color,
                            const unsigned int& texID) {
    drawQuad({ pos.x, pos.y }, { pos.x, pos.y + height },
             { pos.x + width, pos.y + height }, { pos.x + width, pos.y },
             color, texID);
  }

  void Renderer2D::drawQuad(const glm::vec2& a, const glm::vec2& b,
                            const glm::vec2& c, const glm::vec2& d,
                            const glm::vec3& color, const unsigned int& texID) {
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

    // TODO: Use memcpy instead on predefined array size
    m_Vertices.push_back({ { points[0].x, points[0].y, 0.0f }, { 1.0f, 1.0f }, (float)texID, color });
    m_Vertices.push_back({ { points[1].x, points[1].y, 0.0f }, { 0.0f, 1.0f }, (float)texID, color });
    m_Vertices.push_back({ { points[2].x, points[2].y, 0.0f }, { 0.0f, 0.0f }, (float)texID, color });
    m_Vertices.push_back({ { points[3].x, points[3].y, 0.0f }, { 1.0f, 0.0f }, (float)texID, color });

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
                            const float& fontSize, const glm::vec3& color) {
    // TODO: For now, the fontsize parameter only dictates the height, fix it
    // to use point size (pt) instead
    glm::vec2 cursorPos = pos;
    float scaling = fontSize / m_Font->getBoundingBoxY();
    scaling = 1.0f;

    for (size_t i = 0; i < text.length(); i++) {
      char c = text[i];
      FontCharacter fontChar = m_Font->getFontChar(c);

      if (c != ' ') {
        glm::vec2 charPos;
        charPos.x = cursorPos.x + (i != 0 ? fontChar.xOffset * scaling : 0); // first char
        charPos.y = cursorPos.y + fontChar.yOffset * scaling - m_Font->getLowestOffsetY() * scaling;

        // TODO: Use memcpy instead on predefined array size
        m_Vertices.push_back({ { charPos.x + fontChar.sizeX * scaling, charPos.y, 0.0f }, { fontChar.xRightTexCoord, fontChar.yTopTexCoord }, (float)2, color });
        m_Vertices.push_back({ { charPos.x, charPos.y, 0.0f }, { fontChar.xLeftTexCoord,  fontChar.yTopTexCoord }, (float)2, color });
        m_Vertices.push_back({ { charPos.x, charPos.y + fontChar.sizeY * scaling, 0.0f }, { fontChar.xLeftTexCoord,  fontChar.yBottomTexCoord }, (float)2, color });
        m_Vertices.push_back({ { charPos.x + fontChar.sizeX * scaling, charPos.y + fontChar.sizeY * scaling, 0.0f }, { fontChar.xRightTexCoord, fontChar.yBottomTexCoord }, (float)2, color });

        // TODO: Dynamic indices
        unsigned int indexOffset = (m_Indices.size() / 6) * 4;
        m_Indices.push_back(0 + indexOffset);
        m_Indices.push_back(1 + indexOffset);
        m_Indices.push_back(2 + indexOffset);
        m_Indices.push_back(0 + indexOffset);
        m_Indices.push_back(2 + indexOffset);
        m_Indices.push_back(3 + indexOffset);
      }

      cursorPos.x += fontChar.xAdvance * scaling;
    }
  }
}

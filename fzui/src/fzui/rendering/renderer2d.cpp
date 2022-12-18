// FZUI
#include "fzui/rendering/renderer2d.hpp"
#include "fzui/data/texture.hpp"

// std
#include <algorithm>
#include <array>
#include <iostream>

// vendor
#include <glad/glad.h>
#include <glm/gtc/constants.hpp>

namespace fz {
  static Texture tex1;
  static Texture tex2;

  Renderer2D::Renderer2D() {
    tex1.loadFromFile("assets/textures/fzcoach.png");
    tex2.loadFromFile("assets/textures/test.png");

    m_Vertices = std::vector<Vertex>();
    m_Indices = std::vector<unsigned int>();
    m_Indices.push_back(0);
    m_Indices.push_back(1);
    m_Indices.push_back(2);
    m_Indices.push_back(0);
    m_Indices.push_back(2);
    m_Indices.push_back(3);

    BufferLayout bufferLayout;
    bufferLayout.addAttribute<float>("Position", 3);
    bufferLayout.addAttribute<float>("TexCoord", 2);
    bufferLayout.addAttribute<float>("TexIndex", 1);

    m_VBO = std::make_unique<VertexBuffer>(VertexBuffer(m_Vertices, GL_DYNAMIC_DRAW));
    m_IBO = std::make_unique<IndexBuffer>(IndexBuffer(m_Indices, GL_DYNAMIC_DRAW));
    m_VAO = std::make_unique<VertexArray>(VertexArray(bufferLayout, *m_VBO, *m_IBO));
    m_VAO->create();

    glEnable(GL_DEPTH_TEST);
  }

  void Renderer2D::begin() {

  }

  void Renderer2D::end() {

    float c = 0.1f;
    glClearColor(c, c, c, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_VAO->bind();
    glBindTextureUnit(0, tex1.getID());
    //glBindTextureUnit(1, tex2.getID());
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * m_Vertices.size(), m_Vertices.data());

    // Renders the batched data
    glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_INT, (void*)0);
    m_VAO->unbind();
    m_Vertices.clear();
  }

  // Draw functions
  void Renderer2D::drawQuad(const glm::vec2& a, const glm::vec2& b,
                const glm::vec2& c, const glm::vec2& d) {
    std::array<glm::vec2, 4> points = { a, b, c, d };

    // 1. Estimate center point as the mean of the points
    glm::vec2 center((a.x + b.x + c.x + d.x) / 4.0f,
                     (a.y + b.y + c.y + d.y) / 4.0f);

    std::sort(points.begin(), points.end(), [center](glm::vec2 u, glm::vec2 v) {
      glm::vec2 uDir = u - center;
      uDir.y = -uDir.y; // flip sign due to screen space coordinates

      glm::vec2 vDir = v - center;
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

      // If uAngle is not less than vAngle, we must then sort by lowest distance
      // if it turns out that the two angles are equal
      if (uAngle == vAngle && glm::length(uDir) < glm::length(vDir)) {
        return true;
      }

      return false;
    });

    // TODO: Use memcpy instead on predefined array size
    m_Vertices.push_back({ { points[0].x, points[0].y, 0.0f }, { 0.0f, 1.0f }, 0.0f });
    m_Vertices.push_back({ { points[1].x, points[1].y, 0.0f }, { 0.0f, 0.0f }, 0.0f });
    m_Vertices.push_back({ { points[2].x, points[2].y, 0.0f }, { 1.0f, 0.0f }, 0.0f });
    m_Vertices.push_back({ { points[3].x, points[3].y, 0.0f }, { 1.0f, 1.0f }, 0.0f });

    // TODO: Dynamic indices
  }
}

#pragma once

// std
#include <string>
#include <unordered_map>

// FZUI
#include "fzui/core.hpp"

// vendor
#include <glad/glad.h>
#include <glm/glm.hpp>

namespace fz {
  enum class ShaderType {
    Vertex = GL_VERTEX_SHADER,
    Fragment = GL_FRAGMENT_SHADER,
    Geometry = GL_GEOMETRY_SHADER,
    TesselControl = GL_TESS_CONTROL_SHADER,
    TesselEval = GL_TESS_EVALUATION_SHADER,
    Compute = GL_COMPUTE_SHADER
  };

  class FZ_API Shader {
    public:
      Shader() {};
      ~Shader() {};

      void bind() const;
      void loadShader(const ShaderType& type, const std::string& path);
      void compileShaders();

      // Uniforms
      void setUniformVec3(const std::string& name, const glm::vec3& value) const;
      void setUniformMat4(const std::string& name, const glm::mat4& value) const;

      // Getters
      unsigned int getID() const;

    private:
      unsigned int m_ID = 0;
      std::string parseShader(const std::string& path);
      std::unordered_map<ShaderType, int> m_ShaderIDs;
  };
}

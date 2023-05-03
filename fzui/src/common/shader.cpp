// FZUI
#include "fzui/data/shader.hpp"

// std
#include <iostream>
#include <fstream>
#include <sstream>

// vendor
#include <glm/gtc/type_ptr.hpp>


namespace fz {

  // ------ Vulkan ------
  std::vector<char> Shader_Vulkan::readFile(const std::string& path) {
    std::string truePath = BASE_DIR + path;
    std::ifstream file(truePath, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("VULKAN SHADER ERROR: Failed to open shader file!");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();
    return buffer;
  }

  // ------ General ------
  void Shader::bind() const {
    glUseProgram(m_ID);
  }

  void Shader::loadShader(const ShaderType &type, const std::string &path) {
    std::string shaderCode = parseShader(path);
    const char* rawShaderCode = shaderCode.c_str();

    // Check if shader type already exists
    auto res = m_ShaderIDs.find(type);

    if (res != m_ShaderIDs.end()) {
      std::cout << "ERROR: Attempted to add already existing shader type!" << std::endl;
      return;
    }

    // Create a shader of the given type
    unsigned int tempId = glCreateShader((int)type);
    glShaderSource(tempId, 1, &rawShaderCode, NULL);
    glCompileShader(tempId);

    // Print compile errors (if any)
    int success;
    char infoLog[512];

    glGetShaderiv(tempId, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(tempId, 512, NULL, infoLog);
      std::cout << "ERROR: Shader compilation failed!\n" << infoLog << std::endl;

    };

    m_ShaderIDs.insert({ type, tempId });
  }

  void Shader::compileShaders() {
    m_ID = glCreateProgram();

    for (auto shader : m_ShaderIDs) {
      glAttachShader(m_ID, shader.second);
    }

    glLinkProgram(m_ID);

    // Print shader linking errors (if any)
    int success;
    char infoLog[512];
    glGetProgramiv(m_ID, GL_LINK_STATUS, &success);

    if(!success) {
      glGetProgramInfoLog(m_ID, 512, NULL, infoLog);
      std::cout << "ERROR: Shader program linking failed!\n" << infoLog << std::endl;
    }

    // TODO: Refine shader deletion cleanup
    for (auto shader : m_ShaderIDs) {
      glDeleteShader(shader.second);
    }
  }

  // Uniforms
  // TODO: Add automatic binding
  void Shader::setUniformVec2(const std::string& name, const glm::vec2& value) const {
    glUniform2fv(glGetUniformLocation(m_ID, name.c_str()), 1, glm::value_ptr(value));
  }

  void Shader::setUniformVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(m_ID, name.c_str()), 1, glm::value_ptr(value));
  }

  void Shader::setUniformMat4(const std::string& name, const glm::mat4& value) const {
    glUniformMatrix4fv(glGetUniformLocation(m_ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
  }

  std::string Shader::parseShader(const std::string& path) {
    std::ifstream shaderFile;
    shaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    try 
    {
      // open files
      shaderFile.open(BASE_DIR + path);
      std::stringstream shaderStream;
      shaderStream << shaderFile.rdbuf(); // read into string stream
      shaderFile.close();

      return shaderStream.str();
    }
    catch(std::ifstream::failure e)
    {
      std::cout << "ERROR: Shader file could not be succesfully read!" << std::endl;
    }

    return "";
  }

  unsigned int Shader::getID() const {
    return m_ID;
  }
}

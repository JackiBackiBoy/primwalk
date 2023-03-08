// FZUI
#include "fzui/windows/rendering/bufferLayout.hpp"

// vendor
#include <glad/glad.h>

namespace fz {
  int BufferLayout::getStride() const {
    return m_Stride;
  }

  std::vector<BufferAttribute> BufferLayout::getAttributes() const {
    return m_Attributes;
  }

  std::unordered_map<std::type_index, unsigned int> BufferLayout::glTypes = {
    { std::type_index(typeid(float)), GL_FLOAT },
    { std::type_index(typeid(double)), GL_DOUBLE },
    { std::type_index(typeid(char)), GL_BYTE },
    { std::type_index(typeid(unsigned char)), GL_UNSIGNED_BYTE },
    { std::type_index(typeid(int)), GL_INT },
    { std::type_index(typeid(unsigned int)), GL_UNSIGNED_INT },
    { std::type_index(typeid(short)), GL_SHORT },
    { std::type_index(typeid(unsigned short)), GL_UNSIGNED_SHORT }
  };
}

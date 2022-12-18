#pragma once

// std
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

// FZUI
#include "fzui/core/core.hpp"

namespace fz {
  struct FZ_API BufferAttribute {
    std::string name;
    unsigned int glType;
    unsigned int count;
    unsigned int byteSize;
  };

  class FZ_API BufferLayout {
    public:
      BufferLayout() {};
      ~BufferLayout() {};

      template <typename T>
      void addAttribute(const std::string& name, const unsigned int& count) {
        auto res = glTypes.find(std::type_index(typeid(T)));
        
        // Check if attribute type is a valid type to use
        if (res == glTypes.end()) {
          return;
        }

        unsigned int size = (unsigned int)sizeof(T) * count;

        m_Attributes.push_back({ name, res->second, count, size});
        m_Stride += size;
      }

      // Getters
      int getStride() const;
      std::vector<BufferAttribute> getAttributes() const;

    private:
      int m_Stride = 0;
      std::vector<BufferAttribute> m_Attributes;

      static std::unordered_map<std::type_index, unsigned int> glTypes;
  };
}

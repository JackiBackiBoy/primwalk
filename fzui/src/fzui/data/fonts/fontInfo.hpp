#pragma once

// std
#include <string>
#include <functional>

// FZUI
#include "fzui/core/core.hpp"

namespace fz {
  struct FZ_API FontInfo {
    FontInfo() : size(0), flags(0), name("") {};
    ~FontInfo() = default;

    int size;
    int flags;
    std::string name;

    bool operator==(const FontInfo &other) const
    { return (size == other.size
              && flags == other.flags
              && name == other.name);
    }
  };
}

namespace std {
  template <>
  struct FZ_API hash<fz::FontInfo>
  {
    std::size_t operator()(const fz::FontInfo& k) const
    {
      using std::size_t;
      using std::hash;
      using std::string;

      // Compute individual hash values for first,
      // second and third and combine them using XOR
      // and bit shifting:
      return ((hash<int>()(k.size)
               ^ (hash<int>()(k.flags) << 1)) >> 1)
               ^ (hash<string>()(k.name) << 1);
    }
  };
}

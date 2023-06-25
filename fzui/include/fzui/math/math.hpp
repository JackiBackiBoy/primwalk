#ifndef FZ_MATH_HEADER
#define FZ_MATH_HEADER

// FZUI
#include "fzui/core.hpp" 

// std
#include <cstdint>

namespace fz {
  class FZ_API Math {
    public:
      static float lerp(float a, float b, float percentage);

      static inline uint16_t byteSwap(uint16_t value)
      {
        return (((value & 0x00FF) << 8) | ((value & 0xFF00) >> 8));
      }
      static inline uint32_t byteSwap(uint32_t value)
      {
        return (((value & 0x000000FF) << 24) | ((value & 0x0000FF00) << 8) | ((value & 0x00FF0000) >> 8) | ((value & 0xFF000000) >> 24));
      }

    private:
      Math() = default;
      ~Math() = default;
  };
}
#endif
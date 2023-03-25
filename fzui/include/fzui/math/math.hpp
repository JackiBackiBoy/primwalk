#ifndef FZ_MATH_HEADER
#define FZ_MATH_HEADER

// FZUI
#include "fzui/core.hpp" 

namespace fz {
  class FZ_API Math {
    public:
      static float lerp(const float& a, const float&b, float percentage);

    private:
      Math() = default;
      ~Math() = default;
  };
}
#endif
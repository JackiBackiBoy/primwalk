#ifndef FZ_OSX_MOUSE_HEADER
#define FZ_OSX_MOUSE_HEADER

#include "fzui/core.hpp"
#include <glm/glm.hpp>

namespace fz {
  class FZ_API MouseOSX : public MouseBase {
    public:
      virtual ~MouseOSX() {};
    
      static MouseOSX& Instance();

      virtual glm::vec2 getRelativePos() override;
      virtual glm::vec2 getAbsolutePos() override;
    
    protected:
      MouseOSX() {};

      friend class WindowOSX;
  };
}

#endif
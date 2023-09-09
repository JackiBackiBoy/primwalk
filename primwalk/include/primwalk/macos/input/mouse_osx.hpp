#pragma once

#include "primwalk/core.hpp"
#include <glm/glm.hpp>

namespace pw {
  class PW_API MouseOSX : public MouseBase {
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


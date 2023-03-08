#ifndef FZ_WIN32_MOUSE_HEADER
#define FZ_WIN32_MOUSE_HEADER

#include "fzui/core.hpp"
#include <glm/glm.hpp>

namespace fz {
  class FZ_API MouseWin32 : public MouseBase {
    public:
      virtual ~MouseWin32() {};
    
      static MouseWin32& Instance();

      virtual glm::vec2 getRelativePos() override;
      virtual glm::vec2 getAbsolutePos() override;
    
    protected:
      MouseWin32() {};

      friend class WindowWin32;
  };
}

#endif
#pragma once

// primwalk
#include "primwalk/core.hpp"

// std
#include <cstdint>

// windows
#include <windows.h>

namespace pw {
  class PW_API MessageHandler {
    virtual void onMouseDown() {};

  };
}
// FZUI
#include "fzui/application.hpp"

namespace fz {
  Application& Application::Instance() {
    static Application instance;
    return instance;
  }
}

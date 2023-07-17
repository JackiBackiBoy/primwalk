// primwalk
#include "primwalk/application.hpp"

namespace pw {
  Application& Application::Instance() {
    static Application instance;
    return instance;
  }
}

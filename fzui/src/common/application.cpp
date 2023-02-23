// FZUI
#include "fzui/application.hpp"

namespace fz {
  Application& Application::Instance() {
    static Application instance;
    return instance;
  }

  void Application::setMainWindow(Window* window) {
    m_MainWindow = window;
  }
}

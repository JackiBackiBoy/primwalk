#include "fzui/application.hpp"

namespace fz {
  Application& Application::Instance() {
    static Application instance;
    return instance;
  }

  void Application::setMainWindow(WindowWin32* window) {
    m_MainWindow = window;
  }
}

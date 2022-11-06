#include "fzui/core/application.hpp"

namespace fz {
  Application& Application::Instance() {
    static Application instance;
    return instance;
  }

  Window* Application::addWindow(const std::wstring& title,
      const WindowInfo& info) {
    Window* window = new Window(title, 720, 360);
    window->setWindowInfo(info);

    m_Windows.insert({ title, window });
    return window;
  }

  void Application::setMainWindow(Window* window) {
    m_MainWindow = window;
  }
}

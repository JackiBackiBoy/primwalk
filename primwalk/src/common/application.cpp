// primwalk
#include "primwalk/application.hpp"

#include "primwalk/ui/uiButton.hpp"
#include "primwalk/ui/uiContainer.hpp"
#include "primwalk/ui/uiElement.hpp"
#include "primwalk/ui/uiIconButton.hpp"
#include "primwalk/ui/uiLabel.hpp"
#include "primwalk/ui/uiSlider.hpp"
#include "primwalk/ui/uiTextField.hpp"

// std
#include <thread>

namespace pw {
  void Application::setWindow(Window* window)
  {
    m_Window = window;

    // Create graphics device
    m_Device = std::make_unique<GraphicsDevice_Vulkan>(*m_Window);
    pw::GetDevice() = m_Device.get();

    m_Renderer = std::make_unique<Renderer>(*m_Window);
    VkRenderPass renderPass = m_Renderer->getSwapChainRenderPass();
    std::vector<VkDescriptorSetLayout> layouts = {};
    m_UIRenderSystem = std::make_unique<UIRenderSystem>(*m_Device, renderPass, layouts);

    // Textures
    auto minimizeIcon = Texture2D::create("assets/icons/minimize.png");
    auto maximizeIcon = Texture2D::create("assets/icons/maximize.png");
    auto closeIcon = Texture2D::create("assets/icons/close.png");

    // Fonts
    auto headerFont = pw::Font::create("assets/fonts/opensans.ttf", 32, pw::FontWeight::ExtraBold);

    testLabel.setText("Primwalk");
    m_GUI.addWidget(&testLabel);

    fileMenu.setText("File");
    fileSubNewMenu.setText("New");
    fileSubOpenMenu.setText("Open");
    fileMenu.addItem(&fileSubNewMenu);
    fileMenu.addItem(&fileSubOpenMenu);

    editMenu.setText("Edit");
    assetsMenu.setText("Assets");
    toolsMenu.setText("Tools");
    windowMenu.setText("Window");
    helpMenu.setText("Help");

    menu.addItem(&fileMenu);
    menu.addItem(&editMenu);
    menu.addItem(&assetsMenu);
    menu.addItem(&toolsMenu);
    menu.addItem(&windowMenu);
    menu.addItem(&helpMenu);
    menu.setPosition({ 80, 0 });
    m_GUI.addWidget(&menu);
  }

  void Application::run()
  {
    std::thread renderThread([this]() { onRender(); }); // separate render thread
    m_Window->run(); // window message loop will run until window exit
    renderThread.join();
  }

  void Application::onRender()
  {
    bool firstPaint = true;
    auto lastTime = std::chrono::high_resolution_clock::now();

    // TODO: Make frame timing consistent when resizing window
    while (!m_Window->shouldClose()) {
      auto newTime = std::chrono::high_resolution_clock::now();
      float dt = std::chrono::duration<float, std::chrono::seconds::period>(
        newTime - lastTime).count();
      lastTime = newTime;

      if (auto commandBuffer = m_Renderer->beginFrame()) {
        int frameIndex = m_Renderer->getFrameIndex();

        // Update
        FrameInfo frameInfo{};
        frameInfo.frameIndex = frameIndex;
        frameInfo.frameTime = dt;
        frameInfo.commandBuffer = commandBuffer;
        frameInfo.windowWidth = (float)m_Renderer->getSwapChainWidth();
        frameInfo.windowHeight = (float)m_Renderer->getSwapChainHeight();

        m_GUI.onRender(*m_UIRenderSystem);
        m_UIRenderSystem->onUpdate(frameInfo);

        // Render
        m_Renderer->beginSwapChainRenderPass(commandBuffer);
        m_UIRenderSystem->onRender(frameInfo);
        m_Renderer->endSwapChainRenderPass(commandBuffer);

        m_Renderer->endFrame();
      }

      if (firstPaint) {
        //ShowWindow(m_Window->getHandle(), SW_SHOW);
        //UpdateWindow(m_Window->getHandle());
        firstPaint = false;
      }
    }

    vkDeviceWaitIdle(m_Device->getDevice());
  }
}

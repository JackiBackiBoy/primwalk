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
#include <iostream>

namespace pw {
  // TODO: MAJOR refactor needed
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

    // Subviews
    std::unique_ptr<SubView> sceneView = std::make_unique<SubView>(500, 500, glm::vec2(300, 200));
    m_SubViews.push_back(std::move(sceneView));
    m_RenderSystem3D = std::make_unique<RenderSystem3D>(*m_Device, m_SubViews[0]->m_OffscreenPass->getVulkanRenderPass());

    // Textures
    auto fullscreenIcon = Texture2D::create("assets/icons/fullscreen.png");
    auto minimizeIcon = Texture2D::create("assets/icons/minimize.png");
    auto maximizeIcon = Texture2D::create("assets/icons/maximize.png");
    auto closeIcon = Texture2D::create("assets/icons/close.png");
    auto navbarIcons = Texture2D::create("assets/icons/navbar_icons.png"); // icon atlas

    testLabel.setText("File");
    testLabel.setPosition({ 100, 100 });
    testLabel.setFontSize(12);
    testLabel.setTextColor(Color::White);
    m_GUI.addWidget(&testLabel);

    engineLogo.setImage(Texture2D::create("assets/icons/primwalk_logo.png"));
    engineLogo.setPosition({ m_Window->getWidth() / 2 - 80 / 2, 0 });
    engineLogo.setWidth(80);
    engineLogo.setHeight(30);
    engineLogo.setColor({ 255, 255, 255, 100 });
    m_GUI.addWidget(&engineLogo);

    // Nav-bar editor buttons
    selectButton.setIcon(navbarIcons);
    selectButton.setIconScissor({ 0, 0 }, 32, 32);
    selectButton.setPosition({ 10, 30 });
    selectButton.setWidth(20);
    selectButton.setHeight(20);
    selectButton.setPadding(2);
    selectButton.setBackgroundHoverColor({ 61, 61, 61 });

    moveButton.setIcon(navbarIcons);
    moveButton.setIconScissor({ 32, 0 }, 32, 32);
    moveButton.setPosition({ 42, 30 });
    moveButton.setWidth(20);
    moveButton.setHeight(20);
    moveButton.setPadding(2);
    moveButton.setBackgroundHoverColor({ 61, 61, 61 });

    rotateButton.setIcon(navbarIcons);
    rotateButton.setIconScissor({ 64, 0 }, 32, 32);
    rotateButton.setPosition({ 74, 30 });
    rotateButton.setWidth(20);
    rotateButton.setHeight(20);
    rotateButton.setPadding(2);
    rotateButton.setBackgroundHoverColor({ 61, 61, 61 });

    scaleButton.setIcon(navbarIcons);
    scaleButton.setIconScissor({ 96, 0 }, 32, 32);
    scaleButton.setPosition({ 106, 30 });
    scaleButton.setWidth(20);
    scaleButton.setHeight(20);
    scaleButton.setPadding(2);
    scaleButton.setBackgroundHoverColor({ 61, 61, 61 });

    m_GUI.addWidget(&selectButton);
    m_GUI.addWidget(&moveButton);
    m_GUI.addWidget(&rotateButton);
    m_GUI.addWidget(&scaleButton);

    fullscreenButton.setIcon(fullscreenIcon);
    fullscreenButton.setWidth(32);
    fullscreenButton.setHeight(30);
    fullscreenButton.setPosition({ window->getWidth() - 122, 0 });
    fullscreenButton.setOnClick([this]() { m_Window->toggleFullscreen(); });
    m_GUI.addWidget(&fullscreenButton);

    minimizeButton.setIcon(minimizeIcon);
    minimizeButton.setWidth(30);
    minimizeButton.setHeight(30);
    minimizeButton.setPosition({ window->getWidth() - 90, 0 });
    //minimizeButton.setOnClick([this]() { SendMessage(m_Window->getHandle(), WM_SYSCOMMAND, SC_MINIMIZE, 0); });
    //minimizeButton.setOnClick([this]() { m_Window->toggleFullscreen(); });
    m_GUI.addWidget(&minimizeButton);

    maximizeButton.setIcon(maximizeIcon);
    maximizeButton.setWidth(30);
    maximizeButton.setHeight(30);
    maximizeButton.setPosition({ window->getWidth() - 60, 0 });
    maximizeButton.setOnClick([this]() { m_Window->toggleMaximize(); });
    m_GUI.addWidget(&maximizeButton);

    closeButton.setIcon(closeIcon);
    closeButton.setWidth(30);
    closeButton.setHeight(30);
    closeButton.setPosition({ window->getWidth() - 30, 0 });
    closeButton.setOnClick([this]() { m_Window->close(); });
    closeButton.setBackgroundHoverColor({ 200, 0, 0 });
    closeButton.setBackgroundClickColor({ 128, 0, 0 });
    m_GUI.addWidget(&closeButton);

    fileMenu.setText("File");
    fileSubNewMenu.setText("New");
    fileSubOpenMenu.setText("Open");
    fileMenu.addItem(&fileSubNewMenu);
    fileMenu.addItem(&fileSubOpenMenu);
    fileMenu.addSeparator();

    editMenu.setText("Edit");
    assetsMenu.setText("Assets");
    toolsMenu.setText("Tools");
    windowMenu.setText("Window");

    // ------ Help Menu ------
    helpMenu.setText("Help");
    helpView.setText("View Help");
    helpGettingStarted.setText("Getting Started");
    helpTipsAndTricks.setText("Tips and Tricks");
    helpKeyboardShortcuts.setText("Keyboard Shortcuts");
    helpWhatsNew.setText("What's New?");
    helpCheckForUpdates.setText("Check for Updates");
    helpReleaseNotes.setText("Release Notes");
    helpRoadmap.setText("Roadmap");
    helpAbout.setText("About Primwalk");

    helpMenu.addItem(&helpView);
    helpMenu.addItem(&helpGettingStarted);
    helpMenu.addItem(&helpTipsAndTricks);
    helpMenu.addItem(&helpKeyboardShortcuts);
    helpMenu.addItem(&helpWhatsNew);
    helpMenu.addItem(&helpCheckForUpdates);
    helpMenu.addItem(&helpReleaseNotes);
    helpMenu.addItem(&helpRoadmap);
    helpMenu.addItem(&helpAbout);

    // ------ Construct the complete menu widget ------
    menu.addItem(&fileMenu);
    menu.addItem(&editMenu);
    menu.addItem(&assetsMenu);
    menu.addItem(&toolsMenu);
    menu.addItem(&windowMenu);
    menu.addItem(&helpMenu);
    menu.setPosition({ 20, 7 });
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
      std::unique_lock<std::mutex> lock(m_RenderingMutex);
      while (m_Resizing.load()) {
        m_RenderCondition.wait(lock);
      }

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

        if (m_Window->isFullscreen()) {
          int i = 0;
        }
        frameInfo.windowWidth = (float)m_Window->getWidth();
        frameInfo.windowHeight = (float)m_Window->getHeight();

        for (const auto& view : m_SubViews) {
          FrameInfo subViewInfo = frameInfo;
          subViewInfo.windowWidth = view->getWidth();
          subViewInfo.windowHeight = view->getHeight();

          m_RenderSystem3D->onUpdate(frameInfo);

          view->beginPass(commandBuffer);
          m_RenderSystem3D->onRender(frameInfo);
          view->endPass(commandBuffer);

          m_UIRenderSystem->drawSubView(*view);
        }

        m_GUI.onRender(*m_UIRenderSystem);
        m_UIRenderSystem->onUpdate(frameInfo);

        // Render
        m_Renderer->beginSwapChainRenderPass(commandBuffer);
        m_UIRenderSystem->onRender(frameInfo);
        m_Renderer->endSwapChainRenderPass(commandBuffer);

        m_Renderer->endFrame();
        m_Renderer->m_FramebufferResized.store(false);
      }

      if (firstPaint) {
        ShowWindow(m_Window->getHandle(), SW_SHOW);
        UpdateWindow(m_Window->getHandle());
        firstPaint = false;

        // Window resizing and render synchronization.
        // When a resize occurs, the callback function
        // will notify the renderer of a framebuffer
        // resize and will halt the window messaging
        // thread until the swapchain has been
        // recreated.
        m_Window->setResizeCallback([this](int width, int height) {
          m_Renderer->m_FramebufferResized.store(true);
          m_Resizing.store(true);
          m_RenderingMutex.lock();
          
          fullscreenButton.setPosition({ m_Window->getWidth() - 122, 0 });
          minimizeButton.setPosition({ m_Window->getWidth() - 90, 0 });
          maximizeButton.setPosition({ m_Window->getWidth() - 60, 0 });
          closeButton.setPosition({ m_Window->getWidth() - 30, 0 });
          engineLogo.setPosition({ (m_Window->getWidth() - engineLogo.getWidth()) / 2, engineLogo.getPosition().y });

          m_UIRenderSystem->removeImage(m_SubViews[0]->getImage());
          m_SubViews[0]->setPosition({ width / 4, height / 4 });
          m_SubViews[0]->resize(width / 2, height / 2);

          m_Resizing.store(false);
          m_RenderingMutex.unlock();
          m_RenderCondition.notify_all();
        });
      }

      lock.unlock();
    }

    vkDeviceWaitIdle(m_Device->getDevice());
  }
}

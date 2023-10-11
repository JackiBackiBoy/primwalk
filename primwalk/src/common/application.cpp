#include "primwalk/application.hpp"
#include "primwalk/input/input.hpp"
#include "primwalk/components/camera.hpp"
#include "primwalk/components/renderable.hpp"
#include "primwalk/components/tag.hpp"
#include "primwalk/components/transform.hpp"
#include "primwalk/managers/componentManager.hpp"
#include "primwalk/managers/entityManager.hpp"
#include "primwalk/managers/systemManager.hpp"
#include "primwalk/math/pwmath.hpp"


// std
#include <thread>
#include <iostream>

namespace pw {

  Application::Application()
  {
    // Components
    ComponentManager::Get().registerComponent<Renderable>();
    ComponentManager::Get().registerComponent<Tag>();
    ComponentManager::Get().registerComponent<Transform>();
  }

  void Application::setWindow(Window* window)
  {
    m_Window = window;

    ComponentManager& componentManager = ComponentManager::Get();
    EntityManager& entityManager = EntityManager::Get();
    SystemManager& systemManager = SystemManager::Get();

    m_Device = std::make_unique<GraphicsDevice_Vulkan>(*m_Window);
    pw::GetDevice() = m_Device.get();

    m_Renderer = std::make_unique<Renderer>(*m_Window);
    m_UIRenderSystem = std::make_unique<UIRenderSystem>(*m_Device, m_Renderer->getSwapChainRenderPass());

    m_SceneView = std::make_unique<SubView>(m_Window->getWidth() / 2, m_Window->getHeight() / 2,
      glm::vec2(m_Window->getWidth() / 4, 100));

    m_RenderSystem3D = systemManager.registerSystem<RenderSystem3D>(
      *m_Device, m_SceneView->m_OffscreenPass->getVulkanRenderPass());
    {
      component_signature signature;
      signature.set(componentManager.getComponentType<Renderable>());
      signature.set(componentManager.getComponentType<Transform>());
      systemManager.setSignature<RenderSystem3D>(signature);
    }

    // Models
    m_Cube = std::make_unique<Model>();
    m_Cube->loadFromFile("assets/models/helmet.gltf");

    // Textures
    auto fullscreenIcon = Texture2D::create("assets/icons/fullscreen.png");
    auto minimizeIcon = Texture2D::create("assets/icons/minimize.png");
    auto maximizeIcon = Texture2D::create("assets/icons/maximize.png");
    auto closeIcon = Texture2D::create("assets/icons/close.png");
    icons = Texture2D::create("assets/icons/icons.png"); // icon atlas

    testLabel.setText("test_project.pwproj");
    testLabel.setPosition({ 100, 70 });
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
    selectButton.setIcon(icons);
    selectButton.setIconScissor({ 0, 0 }, 32, 32);
    selectButton.setPosition({ 10, 30 });
    selectButton.setWidth(20);
    selectButton.setHeight(20);
    selectButton.setPadding(2);
    selectButton.setBackgroundHoverColor({ 61, 61, 61 });

    moveButton.setIcon(icons);
    moveButton.setIconScissor({ 32, 0 }, 32, 32);
    moveButton.setPosition({ 42, 30 });
    moveButton.setWidth(20);
    moveButton.setHeight(20);
    moveButton.setPadding(2);
    moveButton.setBackgroundHoverColor({ 61, 61, 61 });

    rotateButton.setIcon(icons);
    rotateButton.setIconScissor({ 64, 0 }, 32, 32);
    rotateButton.setPosition({ 74, 30 });
    rotateButton.setWidth(20);
    rotateButton.setHeight(20);
    rotateButton.setPadding(2);
    rotateButton.setBackgroundHoverColor({ 61, 61, 61 });

    scaleButton.setIcon(icons);
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
    fullscreenButton.setPosition({ window->getWidth() - 140, 0 });
    fullscreenButton.setOnClick([this]() { m_Window->toggleFullscreen(); });
    m_GUI.addWidget(&fullscreenButton);

    minimizeButton.setIcon(minimizeIcon);
    minimizeButton.setWidth(30);
    minimizeButton.setHeight(30);
    minimizeButton.setPosition({ window->getWidth() - 90, 0 });
    minimizeButton.setOnClick([this]() { SendMessage(m_Window->getHandle(), WM_SYSCOMMAND, SC_MINIMIZE, 0); });
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

    m_GUI.addWidget(&pauseButton);

    // Scene explorer
    sceneExplorer.setTitle("Scene Explorer");
    sceneExplorer.setBorderRadius(8);
    sceneExplorer.setPosition({ 4, 100 });
    sceneExplorer.setWidth(m_Window->getWidth() / 4 - 8);
    sceneExplorer.setHeight(m_Window->getHeight() - sceneExplorer.getPosition().y - 4);
    sceneExplorer.setBackgroundColor({ 60, 60, 60 });
    sceneExplorer.setFontSize(12);
    m_GUI.addWidget(&sceneExplorer);

    sceneEntitiesList.setPosition(sceneExplorer.getPosition() + glm::vec2(4, 24 ));
    sceneEntitiesList.setWidth(sceneExplorer.getWidth() - 8);
    sceneEntitiesList.setHeight(m_Window->getHeight() - sceneEntitiesList.getPosition().y - 8);
    sceneEntitiesList.setBackgroundColor({ 40, 40, 40 });
    sceneEntitiesList.addItem({ "Helmet", icons, { 224, 0 }, 32, 32 });
    m_GUI.addWidget(&sceneEntitiesList);

    // Properties panel
    propertiesPanel.setTitle("Properties");
    propertiesPanel.setBorderRadius(8);
    propertiesPanel.setPosition(m_SceneView->getPosition() + glm::vec2(m_SceneView->getWidth() + 4, 0));
    propertiesPanel.setWidth(m_Window->getWidth() / 4 - 8);
    propertiesPanel.setHeight(m_Window->getHeight() - propertiesPanel.getPosition().y - 4);
    propertiesPanel.setBackgroundColor({ 60, 60, 60 });
    m_GUI.addWidget(&propertiesPanel);

    // Scene controls
    playButton.setIcon(icons);
    playButton.setIconScissor({ 128, 0 }, 32, 32);
    playButton.setPosition({ m_Window->getWidth() / 2 - 32, 30 });
    playButton.setWidth(32);
    playButton.setHeight(32);
    playButton.setBackgroundHoverColor({ 61, 61, 61 });
    playButton.setOnClick([this]() { m_ScenePaused = false; });
    m_GUI.addWidget(&playButton);

    pauseButton.setIcon(icons);
    pauseButton.setIconScissor({ 160, 0 }, 32, 32);
    pauseButton.setPosition({ m_Window->getWidth() / 2, 30 });
    pauseButton.setWidth(32);
    pauseButton.setHeight(32);
    pauseButton.setBackgroundHoverColor({ 61, 61, 61 });
    pauseButton.setOnClick([this]() { m_ScenePaused = true; });

    // ------ Construct the complete menu widget ------
    menu.addItem(&fileMenu);
    menu.addItem(&editMenu);
    menu.addItem(&assetsMenu);
    menu.addItem(&toolsMenu);
    menu.addItem(&windowMenu);
    menu.addItem(&helpMenu);
    menu.setPosition({ 20, 7 });
    m_GUI.addWidget(&menu);

    // Entities
    auto cubeEntity = std::make_unique<Entity>("Cube");
    cubeEntity->addComponent<Renderable>().model = m_Cube.get();

    m_Entities.push_back(std::move(cubeEntity));

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
    auto& camera = Camera::MainCamera;

    float speed = 5.0f;
    float orbitDistance = 3.0f;
    glm::vec3 orbitOrigin = { 0, 0, 0 };

    // Rendering loop
    while (!m_Window->shouldClose()) {
      std::unique_lock<std::mutex> lock(m_RenderingMutex);
      m_RenderCondition.wait(lock, [this]() { return !m_Resizing.load(); });

      auto newTime = std::chrono::high_resolution_clock::now();
      float dt = std::chrono::duration<float, std::chrono::seconds::period>(newTime - lastTime).count();
      lastTime = newTime;

      // Input polling
      pw::input::update();
      pw::input::MouseState mouseState;
      pw::input::getMouseState(&mouseState);

      if (auto commandBuffer = m_Renderer->beginFrame()) {
        int frameIndex = m_Renderer->getFrameIndex();

        // Input
        if (mouseState.rightDown) {
          camera->setYaw(camera->getYaw() + mouseState.deltaPosition.x * dt);
          camera->setPitch(camera->getPitch() + mouseState.deltaPosition.y * dt);
        }

        if (pw::input::isDown(KeyCode::MouseButtonMiddle)) {
          // Middle mouse + Left Shift = horizontal/vertical move
          if (pw::input::isDown(KeyCode::KeyboardButtonLShift)) {
            camera->setPosition(camera->getPosition() - camera->getRight() * mouseState.deltaPosition.x * dt * speed);
            camera->setPosition(camera->getPosition() + camera->getUp() * mouseState.deltaPosition.y * dt * speed);
          }
          // Only Middle mouse = orbit move
          else {
            camera->setYaw(camera->getYaw() + mouseState.deltaPosition.x * dt);
            camera->setPitch(camera->getPitch() + mouseState.deltaPosition.y * dt);
            camera->update();
            camera->setPosition(orbitOrigin - camera->getForward() * orbitDistance);
          }
        }
        else {
          if (mouseState.wheelDelta != 0.0f) {
            orbitDistance = std::max(0.01f, orbitDistance - mouseState.wheelDelta * 0.1f * (orbitDistance));
            camera->setPosition(orbitOrigin - camera->getForward() * orbitDistance);
          }
        }

        // Update
        FrameInfo frameInfo{};
        frameInfo.frameIndex = frameIndex;
        frameInfo.frameTime = dt;
        frameInfo.commandBuffer = commandBuffer;
        frameInfo.windowWidth = (float)m_Window->getWidth();
        frameInfo.windowHeight = (float)m_Window->getHeight();

        // Scene view
        FrameInfo subViewInfo = frameInfo;
        subViewInfo.windowWidth = m_SceneView->getWidth();
        subViewInfo.windowHeight = m_SceneView->getHeight();

        if (m_ScenePaused) {
          subViewInfo.frameTime = 0.0f;
        }

        camera->update();
        m_RenderSystem3D->onUpdate(subViewInfo);

        updateScene(dt);
        m_SceneView->beginPass(commandBuffer);
        renderScene(subViewInfo);
        m_SceneView->endPass(commandBuffer);

        m_UIRenderSystem->drawSubView(*m_SceneView);

        // Render GUI
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
          
          fullscreenButton.setPosition({ width - 140, 0 });
          minimizeButton.setPosition({ width - 90, 0 });
          maximizeButton.setPosition({ width - 60, 0 });
          closeButton.setPosition({ width - 30, 0 });
          engineLogo.setPosition({ (width - engineLogo.getWidth()) / 2, engineLogo.getPosition().y });

          playButton.setPosition({ width / 2 - 32, 30 });
          pauseButton.setPosition({ width / 2, 30 });
          sceneExplorer.setWidth(width / 4 - 8);
          sceneExplorer.setHeight(height - sceneExplorer.getPosition().y - 4);
          sceneEntitiesList.setWidth(sceneExplorer.getWidth() - 8);
          sceneEntitiesList.setHeight(height - sceneEntitiesList.getPosition().y - 8);

          propertiesPanel.setPosition({ (3 * width) / 4.0f + 4, 100 });
          propertiesPanel.setWidth(width / 4 - 8);
          propertiesPanel.setHeight(height - sceneExplorer.getPosition().y - 4);

          m_UIRenderSystem->removeImage(m_SceneView->getImage());
          m_SceneView->setPosition({ width / 4, 100 });
          m_SceneView->resize(width / 2, height / 2);

          m_Resizing.store(false);
          m_RenderingMutex.unlock();
          m_RenderCondition.notify_all();
        });
      }

      lock.unlock();
    }

    vkDeviceWaitIdle(m_Device->getDevice());
  }

  void Application::updateScene(float dt)
  {
    sceneEntitiesList.removeAllItems();

    for (const auto& e : m_Entities) {
      sceneEntitiesList.addItem({ e->getComponent<Tag>().name, icons, {224, 0}, 32, 32});
    }
  }

  void Application::renderScene(const FrameInfo& frameInfo)
  {
    m_RenderSystem3D->onRender(frameInfo);
  }

}

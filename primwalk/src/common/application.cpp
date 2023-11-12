#include "application.hpp"
#include "input/input.hpp"
#include "components/camera.hpp"
#include "components/directionLight.hpp"
#include "components/renderable.hpp"
#include "components/pointLight.hpp"
#include "components/tag.hpp"
#include "components/transform.hpp"

#include "math/pwmath.hpp"

// std
#include <thread>
#include <iostream>

namespace pw {

	Application::Application() {
		m_Window = std::make_unique<pw::Window>("Primwalk Engine", 1080, 720);
		m_Device = std::make_unique<GraphicsDevice_Vulkan>(*m_Window);
		pw::GetDevice() = m_Device.get();
		m_Renderer = std::make_unique<Renderer>((GraphicsDevice_Vulkan&)(*m_Device), *m_Window);

		// Components
		m_ComponentManager.registerComponent<Camera>();
		m_ComponentManager.registerComponent<DirectionLight>();
		m_ComponentManager.registerComponent<PointLight>();
		m_ComponentManager.registerComponent<Renderable>();
		m_ComponentManager.registerComponent<Tag>();
		m_ComponentManager.registerComponent<Transform>();

		// Render systems
		m_UIRenderSystem = std::make_unique<UIRenderSystem>((GraphicsDevice_Vulkan&)(*m_Device), m_Renderer->getVkRenderPass());

		/*m_RenderSystem3D = m_SystemManager.registerSystem<RenderSystem3D>(
			(GraphicsDevice_Vulkan&)(*m_Device), m_OffscreenPass->getVulkanRenderPass());
		{
			component_signature signature;
			signature.set(m_ComponentManager.getComponentType<Renderable>());
			signature.set(m_ComponentManager.getComponentType<Transform>());
			m_SystemManager.setSignature<RenderSystem3D>(signature);
		}*/

		m_DeferredPass = std::make_unique<DeferredPass>(m_Window->getWidth() / 2, m_Window->getHeight() / 2, *((GraphicsDevice_Vulkan*)m_Device.get()));

		initialize();

		// Create entities
		Entity* camera = createEntity("Camera");
		camera->getComponent<Transform>().position = { 5.0f, 5.0f, 0.0f };
	}

	Application::~Application() {
	}

	void Application::onStart() {

	}

	void Application::onUpdate(float dt) {

	}

	void Application::onFixedUpdate(float dt) {

	}

	void Application::initialize() {
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
		fullscreenButton.setPosition({ m_Window->getWidth() - 140, 0 });
		m_GUI.addWidget(&fullscreenButton);

		minimizeButton.setIcon(minimizeIcon);
		minimizeButton.setWidth(30);
		minimizeButton.setHeight(30);
		minimizeButton.setPosition({ m_Window->getWidth() - 90, 0 });
		minimizeButton.setOnClick([this]() { SendMessage(m_Window->getHandle(), WM_SYSCOMMAND, SC_MINIMIZE, 0); });
		m_GUI.addWidget(&minimizeButton);

		maximizeButton.setIcon(maximizeIcon);
		maximizeButton.setWidth(30);
		maximizeButton.setHeight(30);
		maximizeButton.setPosition({ m_Window->getWidth() - 60, 0 });
		m_GUI.addWidget(&maximizeButton);

		closeButton.setIcon(closeIcon);
		closeButton.setWidth(30);
		closeButton.setHeight(30);
		closeButton.setPosition({ m_Window->getWidth() - 30, 0 });
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

		sceneEntitiesListBox.setPosition(sceneExplorer.getPosition() + glm::vec2(4, 24 ));
		sceneEntitiesListBox.setWidth(sceneExplorer.getWidth() - 8);
		sceneEntitiesListBox.setHeight(m_Window->getHeight() - sceneEntitiesListBox.getPosition().y - 8);
		sceneEntitiesListBox.setBackgroundColor({ 40, 40, 40 });
		sceneEntitiesListBox.addItem({ "Helmet", icons, { 224, 0 }, 32, 32 });
		m_GUI.addWidget(&sceneEntitiesListBox);

		// Properties panel
		propertiesPanel.setTitle("Properties");
		propertiesPanel.setBorderRadius(8);
		propertiesPanel.setPosition({ (3 * m_Window->getWidth()) / 4 + 4, 100 });
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
	}

	void Application::run() {
		bool firstPaint = true;
		auto lastTime = std::chrono::high_resolution_clock::now();
		float speed = 5.0f;
		auto& camera = Camera::MainCamera;

		ShowWindow(m_Window->getHandle(), SW_SHOW);
		onStart();

		m_Window->setResizeCallback([this](int width, int height) {
			m_Renderer->resizeSwapChain((uint32_t)width, (uint32_t)height);
			fullscreenButton.setPosition({ width - 140, 0 });
			minimizeButton.setPosition({ width - 90, 0 });
			maximizeButton.setPosition({ width - 60, 0 });
			closeButton.setPosition({ width - 30, 0 });
			engineLogo.setPosition({ (width - engineLogo.getWidth()) / 2, engineLogo.getPosition().y });

			playButton.setPosition({ width / 2 - 32, 30 });
			pauseButton.setPosition({ width / 2, 30 });
			sceneExplorer.setWidth(width / 4 - 8);
			sceneExplorer.setHeight(height - sceneExplorer.getPosition().y - 4);
			sceneEntitiesListBox.setWidth(sceneExplorer.getWidth() - 8);
			sceneEntitiesListBox.setHeight(height - sceneEntitiesListBox.getPosition().y - 8);

			propertiesPanel.setPosition({ (3 * width) / 4.0f + 4, 100 });
			propertiesPanel.setWidth(width / 4 - 8);
			propertiesPanel.setHeight(height - sceneExplorer.getPosition().y - 4);

			m_UIRenderSystem->removeImage(m_DeferredPass->getOutputImage());
			m_UIRenderSystem->removeImage(m_DeferredPass->getPositionBufferImage());
			m_UIRenderSystem->removeImage(m_DeferredPass->getNormalBufferImage());
			m_UIRenderSystem->removeImage(m_DeferredPass->getDiffuseBufferImage());

			m_DeferredPass->resize(width / 2, height / 2);
		});

		// Game loop
		while (!m_Window->shouldClose()) {
			// Delta time
			auto newTime = std::chrono::high_resolution_clock::now();
			float dt = std::chrono::duration<float, std::chrono::seconds::period>(newTime - lastTime).count();
			lastTime = newTime;

			// Input polling
			pw::input::update();
			pw::input::KeyboardState keyboard;
			pw::input::MouseState mouseState;
			pw::input::getKeyboardState(&keyboard);
			pw::input::getMouseState(&mouseState);

			// Input
			if (pw::input::isDown(KeyCode::MouseButtonMiddle)) {
				// Middle mouse + Left Shift = horizontal/vertical move
				if (pw::input::isDown(KeyCode::KeyboardButtonLShift)) {
					camera->position += camera->getRight() * mouseState.deltaPosition.x * dt * speed;
					camera->position += camera->getUp() * mouseState.deltaPosition.y * dt * speed;
				}
				// Only Middle mouse = orbit move
				else {
					camera->setYaw(camera->getYaw() - mouseState.deltaPosition.x * dt);
					camera->setPitch(camera->getPitch() - mouseState.deltaPosition.y * dt);
				}
			}
			else {
				if (mouseState.wheelDelta != 0.0f) {
					camera->position += camera->getForward() * (std::signbit(mouseState.wheelDelta) ? -1.0f : 1.0f);
				}
			}

			if (pw::input::isDown(KeyCode::KeyboardButtonLControl) && pw::input::isDownOnce(KeyCode::KeyboardButtonX)) {
				m_DebugMode = !m_DebugMode;
			}

			if (m_ScenePaused) {
				dt = 0.0f;
			}

			camera->update();

			onUpdate(dt);
			onFixedUpdate(dt);

			// Rendering
			onRender(dt);

			m_Window->pollEvents();
		}

		m_Device->waitForGPU();
	}

	Entity* Application::createEntity(const std::string& name) {
		m_Entities.push_back(std::make_unique<Entity>(name, m_ComponentManager, m_EntityManager, m_SystemManager));

		m_DeferredPass->m_Entities.insert((*(m_Entities.end() - 1))->getID());

		return (*(m_Entities.end() - 1)).get();
	}

	Entity* Application::createLightEntity(const std::string& name) {
		auto entity = std::make_unique<Entity>(name, m_ComponentManager, m_EntityManager, m_SystemManager);
		entity->addComponent<PointLight>().color = { 1.0f, 1.0f, 1.0f, 0.1f };
		m_Entities.push_back(std::move(entity));

		m_DeferredPass->m_Entities.insert((*(m_Entities.end() - 1))->getID());

		return (*(m_Entities.end() - 1)).get();
	}

	void Application::onRender(float dt) {
		// Begin command list
		auto commandBuffer = m_Renderer->beginFrame();

		{
			size_t frameIndex = m_Renderer->getFrameIndex();

			// Update
			FrameInfo frameInfo{};
			frameInfo.frameIndex = static_cast<int>(frameIndex);
			frameInfo.frameTime = dt;
			frameInfo.commandBuffer = commandBuffer;
			frameInfo.windowWidth = static_cast<float>(m_Window->getWidth());
			frameInfo.windowHeight = static_cast<float>(m_Window->getHeight());

			// Deferred rendering
			m_DeferredPass->draw(commandBuffer, frameIndex, m_ComponentManager);

			// Render GUI
			updateScene(dt);
			m_GUI.onRender(*m_UIRenderSystem);

			// Main renderpass (swapchain)
			m_Renderer->beginRenderPass(commandBuffer);
			m_UIRenderSystem->drawFramebuffer(m_DeferredPass->getOutputImage(), { m_Window->getWidth() / 4, 100 });

			// Draw G-Buffer resources
			float aspect =  (float)m_DeferredPass->getOutputImage()->getHeight() / m_DeferredPass->getOutputImage()->getWidth();
			int elemWidth = (m_Window->getWidth() / 2) / 3;
			int elemHeight = elemWidth * aspect;
			glm::vec2 groupOrigin = { m_Window->getWidth() / 4, 100 + m_DeferredPass->getOutputImage()->getHeight() };

			m_UIRenderSystem->drawFramebuffer(m_DeferredPass->getPositionBufferImage(), groupOrigin, elemWidth, elemHeight);
			m_UIRenderSystem->drawFramebuffer(m_DeferredPass->getNormalBufferImage(), groupOrigin + glm::vec2(elemWidth, 0), elemWidth, elemHeight);
			m_UIRenderSystem->drawFramebuffer(m_DeferredPass->getDiffuseBufferImage(), groupOrigin + glm::vec2(elemWidth * 2, 0), elemWidth, elemHeight);

			m_UIRenderSystem->onUpdate(frameInfo);
			m_UIRenderSystem->onRender(frameInfo);
			m_Renderer->endRenderPass(commandBuffer);
		}

		m_Renderer->endFrame();
	}

	void Application::updateScene(float dt) {
		sceneEntitiesListBox.removeAllItems();

		for (const auto& e : m_Entities) {
			sceneEntitiesListBox.addItem({
				e->getComponent<Tag>().name,
				icons, {224, 0}, 32, 32,
				[this]() {}});
		}
	}

}

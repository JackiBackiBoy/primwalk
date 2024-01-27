#include "application.hpp"
#include "input/input.hpp"
#include "components/camera.hpp"
#include "components/directionLight.hpp"
#include "components/renderable.hpp"
#include "components/pointLight.hpp"
#include "components/tag.hpp"
#include "components/transform.hpp"
#include "ui/editor.hpp"

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

		// Renderpasses
		m_GBufferPass = std::make_unique<GBufferPass>(m_Window->getWidth() / 2, m_Window->getHeight() / 2, *((GraphicsDevice_Vulkan*)m_Device.get()));
		m_ShadowPass = std::make_unique<ShadowPass>(*((GraphicsDevice_Vulkan*)m_Device.get()), 1024);
		m_LightingPass = std::make_unique<LightingPass>(m_Window->getWidth() / 2, m_Window->getHeight() / 2, *((GraphicsDevice_Vulkan*)m_Device.get()));

		initialize();
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
		// Create base-entities
		Entity* camera = createEntity("Camera");
		camera->getComponent<Transform>().position = { 5.0f, 5.0f, 0.0f };

		Editor::getInstance().setTarget(this);
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

			m_UIRenderSystem->removeImage(m_GBufferPass->getPositionBuffer());
			m_UIRenderSystem->removeImage(m_GBufferPass->getNormalBuffer());
			m_UIRenderSystem->removeImage(m_GBufferPass->getAlbedoBuffer());
			m_UIRenderSystem->removeImage(m_LightingPass->getOutputImage());

			m_GBufferPass->resize(width / 2, height / 2);
			m_LightingPass->resize(width / 2, height / 2);
		});

		// Game loop
		while (!m_Window->shouldClose()) {
			// Delta time
			auto newTime = std::chrono::high_resolution_clock::now();
			float dt = std::chrono::duration<float, std::chrono::seconds::period>(newTime - lastTime).count();
			lastTime = newTime;

			// Input polling
			pw::input::KeyboardState keyboard{};
			pw::input::MouseState mouseState{};

			pw::input::update();
			pw::input::getKeyboardState(&keyboard);
			pw::input::getMouseState(&mouseState);

			// Input
			if (pw::input::isDown(KeyCode::MouseButtonMiddle)) {
				// Middle mouse + Left Shift = horizontal/vertical move
				if (pw::input::isDown(KeyCode::KeyboardButtonLShift)) {
					camera->position += -camera->getRight() * mouseState.deltaPosition.x * dt * speed;
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

			camera->update(m_Window->getWidth(), m_Window->getHeight());

			onUpdate(dt);
			onFixedUpdate(dt);

			// Rendering
			onRender(dt);

			m_Window->pollEvents();
		}

		m_Device->waitForGPU();
		Editor::getInstance().destroy();
	}

	Entity* Application::createEntity(const std::string& name) {
		m_Entities.push_back(std::make_unique<Entity>(name, m_ComponentManager, m_EntityManager));

		m_GBufferPass->m_Entities.insert((*(m_Entities.end() - 1))->getID());

		return (*(m_Entities.end() - 1)).get();
	}

	Entity* Application::createLightEntity(const std::string& name) {
		auto entity = std::make_unique<Entity>(name, m_ComponentManager, m_EntityManager);
		entity->addComponent<PointLight>().color = { 1.0f, 1.0f, 1.0f, 0.1f };
		m_Entities.push_back(std::move(entity));

		m_GBufferPass->m_Entities.insert((*(m_Entities.end() - 1))->getID());

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

			// Renderpasses (TODO: Proper render graph)
			m_GBufferPass->draw(commandBuffer, frameIndex, m_ComponentManager);
			m_ShadowPass->draw(commandBuffer, frameIndex, m_GBufferPass->m_Entities, m_ComponentManager);
			m_LightingPass->draw(commandBuffer, frameIndex, m_GBufferPass->m_Entities, m_ComponentManager,
				m_GBufferPass->getPositionBuffer(),
				m_GBufferPass->getNormalBuffer(),
				m_GBufferPass->getAlbedoBuffer(),
				m_ShadowPass->getOutputImage(),
				m_ShadowPass->getLightSpaceMatrix()
			);

			// Main renderpass (swapchain)
			m_Renderer->beginRenderPass(commandBuffer);
			m_UIRenderSystem->drawFramebuffer(m_LightingPass->getOutputImage(), { m_Window->getWidth() / 4, 0 });

			// Draw G-Buffer resources
			float aspect =  static_cast<float>(m_LightingPass->getOutputImage()->getHeight()) / m_LightingPass->getOutputImage()->getWidth();
			int elemWidth = (m_Window->getWidth() / 2) / 3;
			int elemHeight = elemWidth * aspect;
			glm::vec2 groupOrigin = { m_Window->getWidth() / 4, 100 + m_LightingPass->getOutputImage()->getHeight() };

			m_UIRenderSystem->drawFramebuffer(m_GBufferPass->getPositionBuffer(), groupOrigin, elemWidth, elemHeight);
			m_UIRenderSystem->drawFramebuffer(m_GBufferPass->getNormalBuffer(), groupOrigin + glm::vec2(elemWidth, 0), elemWidth, elemHeight);
			m_UIRenderSystem->drawFramebuffer(m_GBufferPass->getAlbedoBuffer(), groupOrigin + glm::vec2(elemWidth * 2, 0), elemWidth, elemHeight);
			m_UIRenderSystem->drawFramebuffer(m_ShadowPass->getOutputImage(), groupOrigin + glm::vec2(elemWidth * 3, 0), 256, 256);

			// Editor UI
			Editor::getInstance().draw(*m_UIRenderSystem);

			m_UIRenderSystem->onUpdate(frameInfo);
			m_UIRenderSystem->onRender(frameInfo);
			m_Renderer->endRenderPass(commandBuffer);
		}

		m_Renderer->endFrame();
	}

}

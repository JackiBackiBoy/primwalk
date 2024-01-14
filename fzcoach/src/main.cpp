#define NOMINMAX

// std
#include <iostream>
#include <memory>

// primwalk
#include "primwalk.hpp"

class Sandbox final : public pw::Application {
public:
	Sandbox() : pw::Application() {};

	void onStart() override {
		// Resources
		planeModel = std::make_shared<pw::Model>();
		planeModel->loadFromFile("assets/models/plane.gltf");
		playerModel = std::make_shared<pw::Model>();
		playerModel->loadFromFile("assets/models/helmet.gltf");

		// Entities
		player = createEntity("Player");
		player->getComponent<pw::Renderable>().model = playerModel.get();

		ground = createEntity("Ground");
		ground->getComponent<pw::Transform>().position.y = -1.0f;
		ground->getComponent<pw::Renderable>().model = planeModel.get();

		test = createEntity("Test");

		//light = createLightEntity("Main light");
		//light->getComponent<pw::PointLight>().color = { 1.0f, 1.0f, 1.0f, 4.0f };

		//auto testLight = createLightEntity("Test Light");
		//testLight->getComponent<pw::Transform>().position = { 1.0f, 1.0f, 1.0f };
		//testLight->getComponent<pw::PointLight>().color = { 1.0f, 0.0f, 0.0f, 3.0f };

		auto obj = createEntity("Sun");
		obj->addComponent<pw::DirectionLight>();
	}

	void onUpdate(float dt) override {
		static float t = 0.0f;
		t += dt;

		pw::input::KeyboardState keyboard;
		pw::input::getKeyboardState(&keyboard);

		auto& playerTransform = player->getComponent<pw::Transform>();

		if (pw::input::isDown(pw::KeyCode::KeyboardButtonW)) {
			playerTransform.position.z += 10.0f * dt;
		}
		if (pw::input::isDown(pw::KeyCode::KeyboardButtonA)) {
			playerTransform.position.x -= 10.0f * dt;
		}
		if (pw::input::isDown(pw::KeyCode::KeyboardButtonS)) {
			playerTransform.position.z -= 10.0f * dt;
		}
		if (pw::input::isDown(pw::KeyCode::KeyboardButtonD)) {
			playerTransform.position.x += 10.0f * dt;
		}

		if (pw::input::isDown(pw::KeyCode::KeyboardButtonSpace)) {
			playerTransform.position.y += 10.0f * dt;
		}

		//light->getComponent<pw::Transform>().position = { cos(t), 2.0f, sin(t) };
	}

	void onFixedUpdate(float dt) override {

	}

private:
	pw::Entity* player = nullptr;
	pw::Entity* ground = nullptr;
	pw::Entity* test = nullptr;
	pw::Entity* light = nullptr;
	std::shared_ptr<pw::Model> planeModel = nullptr;
	std::shared_ptr<pw::Model> playerModel = nullptr;
};

int main() {
	Sandbox* application = new Sandbox();
	application->run();
	delete application;

	return 0;
}
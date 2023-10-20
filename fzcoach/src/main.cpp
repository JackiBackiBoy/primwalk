#define NOMINMAX

// std
#include <iostream>
#include <memory>

// primwalk
#include "primwalk/primwalk.hpp"

class Sandbox final : public pw::Application {
public:
	Sandbox() : pw::Application() {};

	void onStart() override {
		// Resources
		planeModel = std::make_shared<pw::Model>();
		planeModel->loadFromFile("assets/models/plane.gltf");

		// Entities
		player = createEntity("Player");

		ground = createEntity("Ground");
		ground->getComponent<pw::Transform>().position.y = -1.0f;
		ground->getComponent<pw::Renderable>().model = planeModel.get();
		ground->getComponent<pw::Renderable>().color = { 100, 100, 100 };

		createEntity("2");
		createEntity("3");
		createEntity("4");
	}

	void onUpdate(float dt) override {
		pw::input::KeyboardState keyboard;
		pw::input::getKeyboardState(&keyboard);

		auto& playerTransform = player->getComponent<pw::Transform>();

		if (pw::input::isDown(pw::KeyCode::KeyboardButtonW)) {
			playerTransform.position.x += 10.0f * dt;
		}
		if (pw::input::isDown(pw::KeyCode::KeyboardButtonS)) {
			playerTransform.position.x -= 10.0f * dt;
		}
		if (pw::input::isDown(pw::KeyCode::KeyboardButtonA)) {
			playerTransform.position.z -= 10.0f * dt;
		}
		if (pw::input::isDown(pw::KeyCode::KeyboardButtonD)) {
			playerTransform.position.z += 10.0f * dt;
		}
	}

	void onFixedUpdate(float dt) override {

	}

private:
	pw::Entity* player = nullptr;
	pw::Entity* ground = nullptr;
	std::shared_ptr<pw::Model> planeModel;
};

int main() {
	Sandbox* application = new Sandbox();
	application->run();
	delete application;

	return 0;
}
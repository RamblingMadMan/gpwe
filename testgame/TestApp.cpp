#include "gpwe/input.hpp"
#include "gpwe/sys.hpp"
#include "gpwe/Shape.hpp"

#include "glm/glm.hpp"

#include "TestApp.hpp"

using namespace gpwe;

GPWE_APP(TestApp, "TESTGAME", "RamblingMad", 0, 0, 0)

TestApp::TestApp(){
	sys::camera()->translate({ 0.f, 0.f, -2.f });

	guyMdl.loadModel("./Assets/Models/SphereGuy.fbx");
	shapes::Cube cube(2.f);

	cubeGroup = sys::renderer()->createGroup(&cube);
	cubeGroup->setNumInstances(0);

	guyGroup = sys::renderer()->createGroup(guyMdl.shape(0));

	auto inputManager = sys::inputManager();

	inputManager->onPumpEvents([this]{
		rot = { 0.f, 0.f, 0.f };
	});

	inputManager->system()->onExitEvent(sys::exit);

	auto kb = inputManager->keyboard();

	kb->onKeyEvent([this](input::Key key, bool pressed){
		using Key = input::Key;

		switch(key){
			case Key::w:{
				movement.z += pressed ? 1.f : -1.f;
				break;
			}

			case Key::s:{
				movement.z -= pressed ? 1.f : -1.f;
				break;
			}

			case Key::a:{
				movement.x -= pressed ? 1.f : -1.f;
				break;
			}

			case Key::d:{
				movement.x += pressed ? 1.f : -1.f;
				break;
			}

			case Key::space:{
				movement.y += pressed ? 1.f : -1.f;
				break;
			}

			case Key::lctrl:{
				movement.y -= pressed ? 1.f : -1.f;
				break;
			}

			case Key::escape:{
				sys::inputManager()->system()->exitEvent();
				break;
			}

			default: break;
		}
	});

	auto mouse = inputManager->mouse();

	mouse->onButtonEvent([this](input::MouseButton btn, bool pressed){
		using Button = input::MouseButton;

		if(btn == Button::left){
			rotateCam = pressed;
		}
	});

	mouse->onMoveEvent([this](std::int32_t xrel, std::int32_t yrel){
		if(!rotateCam) return;

		rot.x += xrel;
		rot.y += yrel;
	});
}

TestApp::~TestApp(){}

void TestApp::update(float dt){
	auto cam = sys::camera();

	auto moveForward = movement.z * cam->forward();
	moveForward.y = 0.f;

	auto moveRight = movement.x * cam->right();
	moveRight.y = 0.f;

	auto moveUp = glm::vec3(0.f, movement.y, 0.f);

	auto moveAmnt = moveForward + moveRight + moveUp;

	if(glm::length(moveAmnt) > 0.f){
		cam->translate(glm::normalize(moveAmnt) * moveSpeed * dt);
	}

	if(rotateCam){
		rot *= rotSpeed * dt;
		cam->rotate(glm::radians(-rot.x), glm::vec3(0.f, 1.f, 0.f));
		cam->rotate(glm::radians(rot.y), glm::vec3(1.f, 0.f, 0.f));
	}
}

#include "gpwe/input.hpp"
#include "gpwe/sys.hpp"
#include "gpwe/resource.hpp"
#include "gpwe/render.hpp"
#include "gpwe/log.hpp"
#include "gpwe/Shape.hpp"

#include "glm/glm.hpp"

#include "TestApp.hpp"

using namespace gpwe;

GPWE_APP(TestApp, "TESTGAME", "RamblingMad", 0, 0, 0)

TestApp::TestApp(){}

TestApp::~TestApp(){}

void TestApp::init(){
	auto resources = sys::resourceManager();
	auto inputs = sys::inputManager();

	shapes::Cube cube(2.f);
	auto terrainMap = HeightMapShape::createSimpleTerrain();
	auto terrainMesh = terrainMap.generateMesh(2.f);
	terrainGroup = sys::renderManager()->createGroup(&terrainMesh);
	terrainInst = terrainGroup->create<gpwe::render::Instance>();

	cubeGroup = sys::renderManager()->createGroup(&cube);

	auto guyMdl = resources->openModel("/Assets/Models/SphereGuy.fbx");
	if(guyMdl){
		guyGroup = sys::renderManager()->createGroup(&guyMdl->meshes()[0]);
	}
	else{
		log::warnLn("could not open '/Assets/Models/SphereGuy.fbx'");
	}

	inputs->onPumpEvents([this]{
		rot = { 0.f, 0.f, 0.f };
	});

	inputs->system()->onExitEvent(sys::exit);

	auto &&kb = inputs->managed<input::Keyboard>().front();

	kb->keyEvent().addFn([this](input::Key key, bool pressed){
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

	auto &&mouse = inputs->managed<input::Mouse>().front();

	mouse->buttonEvent().addFn([this](input::MouseButton btn, bool pressed){
		using Button = input::MouseButton;

		if(btn == Button::left){
			rotateCam = pressed;
		}
	});

	mouse->moveEvent().addFn([this](std::int32_t xrel, std::int32_t yrel){
		if(!rotateCam) return;

		rot.x += xrel;
		rot.y += yrel;
	});
}

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
		rot *= float(2.0 * M_PI) * rotSpeed * dt;
		cam->rotate(glm::radians(-rot.x), glm::vec3(0.f, 1.f, 0.f));
		cam->rotate(glm::radians(rot.y), glm::vec3(1.f, 0.f, 0.f));
	}
}

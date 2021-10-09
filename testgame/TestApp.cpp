#include "gpwe/sys.hpp"
#include "gpwe/Shape.hpp"

#include "TestApp.hpp"

using namespace gpwe;

extern "C"
std::unique_ptr<gpwe::App> gpweCreateApp_test(){
	return std::make_unique<TestApp>();
}

TestApp::TestApp(){
	shapes::Cube cube(2.f);
	cubeGroup = sys::renderer()->createGroup(&cube);
}

TestApp::~TestApp(){}

void TestApp::update(float dt){
}

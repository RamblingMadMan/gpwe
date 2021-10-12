#ifndef TESTGAME_TESTAPP_HPP
#define TESTGAME_TESTAPP_HPP 1

#include "gpwe/Camera.hpp"
#include "gpwe/Renderer.hpp"
#include "gpwe/App.hpp"

class TestApp: public gpwe::App{
	public:
		TestApp();
		~TestApp();

		void update(float dt) override;

	private:
		gpwe::RenderGroup *cubeGroup, *guyGroup;
		bool rotateCam = false;
		glm::vec3 rot = { 0.f, 0.f, 0.f }, movement = { 0.f, 0.f, 0.f };

		float rotSpeed = 2.f;
		float moveSpeed = 2.f;
};

#endif // !TESTGAME_TESTAPP_HPP

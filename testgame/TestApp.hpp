#ifndef TESTGAME_TESTAPP_HPP
#define TESTGAME_TESTAPP_HPP 1

#include "gpwe/Camera.hpp"
#include "gpwe/render.hpp"
#include "gpwe/app.hpp"

class TestApp: public gpwe::app::Manager{
	public:
		TestApp();
		~TestApp();

		void init() override;

		void update(float dt) override;

	private:
		gpwe::render::Group *terrainGroup, *cubeGroup, *guyGroup;
		bool rotateCam = false;
		glm::vec3 rot = { 0.f, 0.f, 0.f }, movement = { 0.f, 0.f, 0.f };

		float rotSpeed = 2.f;
		float moveSpeed = 2.f;
};

#endif // !TESTGAME_TESTAPP_HPP

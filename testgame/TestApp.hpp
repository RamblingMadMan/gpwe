#ifndef TESTGAME_TESTAPP_HPP
#define TESTGAME_TESTAPP_HPP 1

#include "gpwe/Renderer.hpp"
#include "gpwe/App.hpp"

class TestApp: public gpwe::App{
	public:
		TestApp();
		~TestApp();

		std::string_view name() const noexcept override{ return "TESTGAME"; }
		std::string_view author() const noexcept override{ return "RamblingMad"; }
		gpwe::Version version() const noexcept override{ return { 0, 0, 0 }; }

		void update(float dt) override;

	private:
		gpwe::RenderGroup *cubeGroup;
};

#endif // !TESTGAME_TESTAPP_HPP

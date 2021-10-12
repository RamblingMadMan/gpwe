#ifndef GPWE_SYS_HPP
#define GPWE_SYS_HPP 1

#include <cstdint>
#include <functional>

#include "Allocator.hpp"

namespace gpwe{
	class Camera;
	class Renderer;
}

namespace gpwe::input{
	class Manager;
}

namespace gpwe::resource{
	class Manager;
}

namespace gpwe::sys{
	using PresentFn = std::function<void()>;

	void initSys(int argc, char *argv[], input::Manager *inputManager_);
	void initRenderer(std::uint16_t w, std::uint16_t h, void *arg = nullptr);
	void initApp();

	void tick(float dt);

	namespace detail{
		void setRunning(bool val);
	}

	int exec(PresentFn presentFn);

	void exit();

	void *alloc(std::size_t n);
	void free(void *ptr);

	//std::uint32_t numRenderers();

	Camera *camera();
	Renderer *renderer();
	input::Manager *inputManager();
	resource::Manager *resourceManager();
}

#endif // !GPWE_SYS_HPP

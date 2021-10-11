#ifndef GPWE_SYS_HPP
#define GPWE_SYS_HPP 1

#include <cstdint>
#include <functional>

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

	void init(
		int argc, char *argv[],
		input::Manager *inputManager_,
		std::uint16_t w, std::uint16_t h
	);

	int exec(PresentFn presentFn, void *rendererArg);
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

#ifndef GPWE_SYS_HPP
#define GPWE_SYS_HPP 1

#include <cstdint>
#include <functional>

#include "Allocator.hpp"

namespace gpwe{
	class Camera;
}

namespace gpwe::app{
	class Manager;
}

namespace gpwe::render{
	class Manager;
}

namespace gpwe::physics{
	class Manager;
}

namespace gpwe::input{
	class Manager;
}

namespace gpwe::resource{
	class Manager;
}

namespace gpwe::sys{
	using RenderManager = render::Manager;
	using PhysicsManager = physics::Manager;
	using InputManager = input::Manager;
	using ResourceManager = resource::Manager;
	using AppManager = app::Manager;

	using PresentFn = std::function<void()>;

	template<typename T>
	using CreateManagerFn = T*(*)();

	void setRendererArg(void *val = nullptr);

	void setRenderManager(UniquePtr<RenderManager> manager);
	void setPhysicsManager(UniquePtr<PhysicsManager> manager);
	void setInputManager(UniquePtr<InputManager> manager);
	void setAppManager(UniquePtr<AppManager> manager);

	void initSys(int argc, char *argv[]);
	void initInput();
	void initRenderer(std::uint16_t w, std::uint16_t h);
	void initApp();

	void tick(float dt);

	namespace detail{
		void setRunning(bool val);
	}

	int exec(PresentFn presentFn);

	void exit();

	//std::uint32_t numRenderers();

	Camera *camera();
	AppManager *appManager();
	RenderManager *renderManager();
	InputManager *inputManager();
	ResourceManager *resourceManager();
}

#endif // !GPWE_SYS_HPP

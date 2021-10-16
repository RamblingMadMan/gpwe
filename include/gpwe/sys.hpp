#ifndef GPWE_SYS_HPP
#define GPWE_SYS_HPP 1

#include <cstdint>
#include <functional>

namespace gpwe{
	// TODO: put these in respective namespaces as 'Manager's
	class Renderer;
	class Camera;
	class App;
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

	void setRendererArg(void *val = nullptr);

	using CreateAppFn = App*(*)();
	using CreateRendererFn = Renderer*(*)(void*);
	using CreatePhysicsFn = PhysicsManager*(*)(void*);

	void setRenderManager(RenderManager *manager);
	void setPhysicsManager(PhysicsManager *manager);
	void setInputManager(InputManager *manager);
	void setAppManager(AppManager *manager);

	void setCreateAppFn(CreateAppFn fn);
	void setCreateRendererFn(CreateRendererFn fn);

	void initSys(int argc, char *argv[], input::Manager *inputManager_);
	void initRenderer(std::uint16_t w, std::uint16_t h);
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

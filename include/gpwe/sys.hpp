#ifndef GPWE_SYS_HPP
#define GPWE_SYS_HPP 1

#include <cstdint>
#include <atomic>
#include <functional>

#include "Vector.hpp"
#include "Manager.hpp"

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
	class Plugin;
}

namespace gpwe::sys{
	class Manager;

	using SysManager = sys::Manager;
	using RenderManager = render::Manager;
	using PhysicsManager = physics::Manager;
	using InputManager = input::Manager;
	using ResourceManager = resource::Manager;
	using AppManager = app::Manager;

	using PresentFn = std::function<void()>;

	class Manager: public gpwe::Manager<Manager, ManagerKind::sys>{
		public:
			Manager();
			virtual ~Manager();

			void setArgs(int argc, char *argv[]){
				m_argc = argc;
				m_argv = argv;
			}

			virtual void update(float dt) override;

			virtual void init() override;

			void setRenderManager(UniquePtr<RenderManager> manager);
			void setPhysicsManager(UniquePtr<PhysicsManager> manager);
			void setInputManager(UniquePtr<InputManager> manager);
			void setAppManager(UniquePtr<AppManager> manager);

			void setRenderSize(std::uint16_t w, std::uint16_t h);

			void setRenderArg(void *arg){
				m_renderArg = arg;
			}

			const Vector<resource::Plugin*> &plugins() const noexcept{ return m_plugins; }

			const Vector<resource::Plugin*> &renderPlugins() const noexcept{ return m_plugins; }
			const Vector<resource::Plugin*> &physicsPlugins() const noexcept{ return m_plugins; }
			const Vector<resource::Plugin*> &inputPlugins() const noexcept{ return m_plugins; }
			const Vector<resource::Plugin*> &appPlugins() const noexcept{ return m_plugins; }

			RenderManager *renderManager() noexcept{ return m_renderManager.get(); }
			PhysicsManager *physicsManager() noexcept{ return m_physicsManager.get(); }
			InputManager *inputManager() noexcept{ return m_inputManager.get(); }
			AppManager *appManager() noexcept{ return m_appManager.get(); }

		private:
			void initBaseLibraries();
			void loadPlugins();

			UniquePtr<RenderManager> m_renderManager;
			UniquePtr<PhysicsManager> m_physicsManager;
			UniquePtr<InputManager> m_inputManager;
			UniquePtr<AppManager> m_appManager;

			Vector<resource::Plugin*> m_plugins;
			Vector<resource::Plugin*> m_renderPlugins;
			Vector<resource::Plugin*> m_physicsPlugins;
			Vector<resource::Plugin*> m_inputPlugins;
			Vector<resource::Plugin*> m_appPlugins;

			int m_argc = 0; char **m_argv = nullptr;

			void *m_renderArg = nullptr;
			std::uint16_t m_rW = 0, m_rH = 0;

			static std::atomic_flag m_initFlag;
	};

	void setSysManager(UniquePtr<SysManager> manager);

	void init(int argc, char *argv[]);

	void tick(float dt);

	namespace detail{
		void setRunning(bool val);
	}

	int exec(PresentFn presentFn);

	void exit();

	//std::uint32_t numRenderers();

	Camera *camera();
	SysManager *sysManager();
	AppManager *appManager();
	RenderManager *renderManager();
	InputManager *inputManager();
	ResourceManager *resourceManager();
}

#define GPWE_SYS(type, name, author, major, minor, patch)\
	GPWE_PLUGIN(Sys, type, name, author, major, minor, patch)

#endif // !GPWE_SYS_HPP

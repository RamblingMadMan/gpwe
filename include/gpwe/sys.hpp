#ifndef GPWE_SYS_HPP
#define GPWE_SYS_HPP 1

#include <cstdint>
#include <atomic>
#include <functional>

#include "util/Vector.hpp"
#include "util/Ticker.hpp"
#include "util/Thread.hpp"
#include "util/WorkQueue.hpp"

#include "Manager.hpp"

namespace gpwe::sys{
	using PresentFn = Fn<void()>;

	class Manager: public gpwe::Manager<Manager, ManagerKind::sys>{
		public:
			template<typename T>
			using Ptr = UniquePtr<T>;

			Manager();
			virtual ~Manager();

			void setArgs(int argc, char *argv[]){
				m_argc = argc;
				m_argv = argv;
			}

			virtual void update(float dt) override;

			virtual void init() override;

			void exit();
			void quit(){ exit(); }

			int exec(PresentFn presentFn);

			void setLogManager(Ptr<LogManager> manager);
			void setRenderManager(Ptr<RenderManager> manager);
			void setPhysicsManager(Ptr<PhysicsManager> manager);
			void setInputManager(Ptr<InputManager> manager);
			void setAppManager(Ptr<AppManager> manager);

			void setRenderSize(std::uint16_t w, std::uint16_t h);

			void setRenderArg(void *arg){
				m_renderArg = arg;
			}

			const Vector<resource::Plugin*> &plugins() const noexcept{ return m_plugins; }

			const Vector<resource::Plugin*> &inputPlugins() const noexcept{ return m_inputPlugins; }
			const Vector<resource::Plugin*> &renderPlugins() const noexcept{ return m_renderPlugins; }
			const Vector<resource::Plugin*> &physicsPlugins() const noexcept{ return m_physicsPlugins; }
			const Vector<resource::Plugin*> &worldPlugins() const noexcept{ return m_worldPlugins; }
			const Vector<resource::Plugin*> &appPlugins() const noexcept{ return m_appPlugins; }

			LogManager *logManager() noexcept{ return m_logManager.get(); }
			InputManager *inputManager() noexcept{ return m_inputManager.get(); }
			RenderManager *renderManager() noexcept{ return m_renderManager.get(); }
			PhysicsManager *physicsManager() noexcept{ return m_physicsManager.get(); }
			WorldManager *worldManager() noexcept{ return m_worldManager.get(); }
			UiManager *uiManager() noexcept{ return m_uiManager.get(); }
			AppManager *appManager() noexcept{ return m_appManager.get(); }

		private:
			template<bool YieldLoop = false, typename ManagerT>
			void threadFn(UniquePtr<ManagerT> &m, WorkQueue &work){
				while(!m_running){
					work.doWorkOr(std::this_thread::yield);
				}

				Ticker ticker;

				while(m_running){
					auto dt = ticker.tick();
					m->update(dt);
					if constexpr(YieldLoop){
						work.doWorkOr(std::this_thread::yield);
					}
					else{
						work.doWork();
					}
				}

				m.reset();
			}

			void initBaseLibraries();
			void loadPlugins();

			Ptr<LogManager> m_logManager;
			Ptr<RenderManager> m_renderManager;
			Ptr<PhysicsManager> m_physicsManager;
			Ptr<InputManager> m_inputManager;
			Ptr<WorldManager> m_worldManager;
			Ptr<UiManager> m_uiManager;
			Ptr<AppManager> m_appManager;

			Vector<resource::Plugin*> m_plugins;
			Vector<resource::Plugin*> m_logPlugins;
			Vector<resource::Plugin*> m_renderPlugins;
			Vector<resource::Plugin*> m_physicsPlugins;
			Vector<resource::Plugin*> m_inputPlugins;
			Vector<resource::Plugin*> m_worldPlugins;
			Vector<resource::Plugin*> m_appPlugins;

			enum class ThreadIdx{
				worker, render, physics, app,
				count
			};

			Thread m_threads[(std::size_t)ThreadIdx::count];
			WorkQueue m_workQueues[(std::size_t)ThreadIdx::count];

			int m_argc = 0; char **m_argv = nullptr;

			void *m_renderArg = nullptr;
			std::uint16_t m_rW = 0, m_rH = 0;

			std::atomic_bool m_running = false;

			static std::atomic_flag m_initFlag;
	};

	void setManager(SysManager *manager) noexcept;

	void update(float dt);

	void exit();

	//std::uint32_t numRenderers();

	Camera *camera() noexcept;

	SysManager *manager() noexcept;

	inline SysManager *sysManager() noexcept{ return manager(); }
	ResourceManager *resourceManager() noexcept;
	LogManager *logManager() noexcept;
	InputManager *inputManager() noexcept;
	RenderManager *renderManager() noexcept;
	PhysicsManager *physicsManager() noexcept;
	WorldManager *worldManager() noexcept;
	UiManager *uiManager() noexcept;
	AppManager *appManager() noexcept;
}

namespace gpwe::resource{ inline Manager *manager(){ return sys::resourceManager(); } }
namespace gpwe::input{ inline Manager *manager(){ return sys::inputManager(); } }
namespace gpwe::render{ inline Manager *manager(){ return sys::renderManager(); } }
namespace gpwe::physics{ inline Manager *manager(){ return sys::physicsManager(); } }
namespace gpwe::world{ inline Manager *manager(){ return sys::worldManager(); } }
namespace gpwe::ui{ inline Manager *manager(){ return sys::uiManager(); } }
namespace gpwe::app{ inline Manager *manager(){ return sys::appManager(); } }

#define GPWE_SYS(type, name, author, major, minor, patch)\
	GPWE_PLUGIN(Sys, type, name, author, major, minor, patch)

#endif // !GPWE_SYS_HPP

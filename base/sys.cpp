#include <atomic>
#include <optional>
#include <functional>
#include <chrono>
#include <filesystem>

namespace fs = std::filesystem;

using GPWEProc = void(*)();

#ifdef _WIN32
#define LIB_PREFIX
#define LIB_EXT ".dll"
#else
#define LIB_PREFIX "lib"
#define LIB_EXT ".so"
#endif

#include "FastNoise/FastNoise.h"

#include "gpwe/util/Ticker.hpp"
#include "gpwe/util/WorkQueue.hpp"

#include "gpwe/config.hpp"
#include "gpwe/log.hpp"
#include "gpwe/input.hpp"
#include "gpwe/sys.hpp"
#include "gpwe/render.hpp"
#include "gpwe/physics.hpp"
#include "gpwe/app.hpp"
#include "gpwe/resource.hpp"
#include "gpwe/Camera.hpp"

#include "physfs.h"

#include "magic.h"

#include "ft2build.h"
#include FT_FREETYPE_H

#include "FreeImage.h"

#include "glm/glm.hpp"

#include "assimp/version.h"

using namespace gpwe;

FT_Library gpweFtLib = nullptr;

magic_t gpweMagic = nullptr;

thread_local sys::Manager *gpweSysManager = nullptr;

log::Manager gpweDefaultLogManager;
resource::Manager gpweResourceManager; // special, not a plugin

gpwe::Camera gpweCamera(90.f, 1290.f/720.f);

inline void logHeader(const Str &str){ gpwe::log::out("{:^30}\n\n", str); }

constexpr int apiNameColWidth = 16;
constexpr int apiVersionColWidth = 30;
constexpr int apiVersionWidth = apiNameColWidth + apiVersionColWidth;

static void printVersions(){
	FT_Int ftMaj, ftMin, ftPatch;
	FT_Library_Version(gpweFtLib, &ftMaj, &ftMin, &ftPatch);

	StrView libs[] = {
		"PhysFS",
		"libmagic",
		"Freetype",
		"FreeImage",
		"Assimp"
	};

	PHYSFS_Version pfsVer;
	PHYSFS_getLinkedVersion(&pfsVer);

	Str versions[] = {
		gpwe::format("{}.{}.{}", pfsVer.major, pfsVer.minor, pfsVer.patch),
		gpwe::format("{}", magic_version()),
		gpwe::format("{}.{}.{}", ftMaj, ftMin, ftPatch),
		FreeImage_GetVersion(),
		gpwe::format("{}.{}.{}", aiGetVersionMajor(), aiGetVersionMinor(), aiGetVersionPatch())
	};

	logHeader("Version Info");

	gpwe::log::out("┏{:━^{}}┯{:━^{}}┓\n", "┫ API ┣", apiNameColWidth + 2, "┫ Version ┣", apiVersionColWidth + 2);

	for(std::size_t i = 0; i < std::size(libs); i++){
		auto name = libs[i];
		auto &&version = versions[i];

		gpwe::log::out("┃ {:<{}} │ {:<{}} ┃\n", name, apiNameColWidth, version, apiVersionColWidth);
	}

	gpwe::log::out("┗{:━<{}}┷{:━<{}}┛\n\n", "", apiNameColWidth + 2, "", apiVersionColWidth + 2);
}

void sys::Manager::initBaseLibraries(){
	StrView names[] = {
		"PhysFS",
		"libmagic",
		"Freetype",
		"FreeImage"
	};

	std::function<std::optional<Str>()> fns[] = {
		[argv0{m_argv[0]}]() -> std::optional<Str>{
			if(!PHYSFS_init(argv0)){
				return PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
			}

			PHYSFS_setSaneConfig("Hamsmith", "GPWE", nullptr, 1, 0);

			auto assetPath = fs::current_path() / "Assets";

			if(fs::exists(assetPath) && fs::is_directory(assetPath)){
				PHYSFS_mount(assetPath.c_str(), "/Assets", 0);
			}
			else{
				PHYSFS_mkdir("/Assets");
			}

			return std::nullopt;
		},
		[]() -> std::optional<Str>{
			gpweMagic = magic_open(MAGIC_NO_CHECK_COMPRESS | MAGIC_MIME);
			if(!gpweMagic){
				return format("error in magic_open: {}", magic_error(nullptr));
			}

			std::atexit([]{ magic_close(gpweMagic); });

			if(magic_load(gpweMagic, nullptr) != 0) {
				return format("error in magic_load: {}", magic_error(gpweMagic));
			}

			return std::nullopt;
		},
		[]() -> std::optional<Str>{
			auto err = FT_Init_FreeType(&gpweFtLib);
			if(err != FT_Err_Ok){
				// TODO: get error message
				return "";
			}

			std::atexit([]{ FT_Done_FreeType(gpweFtLib); });
			return std::nullopt;
		},
		[]() -> std::optional<Str>{
			FreeImage_Initialise();
			std::atexit(FreeImage_DeInitialise);
			return std::nullopt;
		}
	};

	logHeader("Initializing Libraries");

	for(std::size_t i = 0; i < std::size(names); i++){
		auto name = names[i];
		auto &&fn = fns[i];

		gpwe::log::out("{:<15}", fmt::format("{}...", name));

		auto res = fn();
		if(res){
			gpwe::log::out("Error");
			if(!res.value().empty()){
				gpwe::log::out(": {}\n", res.value());
			}

			std::exit(1);
		}
		else{
			gpwe::log::out("Done\n");
		}
	}

	gpwe::log::out("\n");
}

sys::Manager::Manager(){}

sys::Manager::~Manager(){
	m_appManager.reset();
	m_renderManager.reset();
	m_physicsManager.reset();
	m_inputManager.reset();
}

void sys::Manager::loadPlugins(){
	auto libIter = fs::directory_iterator();

	auto curPath = fs::current_path();

	gpwe::log::infoLn("Searching for plugins in \"{}\"", curPath.c_str());

	for(auto it = fs::directory_iterator(curPath); it != fs::directory_iterator(); ++it){
		auto path = it->path();
		if(path.parent_path() != curPath || path.extension() != LIB_EXT) continue;

		auto pathStr = path.filename().string<char, std::char_traits<char>, Allocator<char>>();

		auto plugin = sys::resourceManager()->openPlugin(pathStr);
		if(!plugin){
			continue;
		}

		gpwe::log::infoLn("Found plugin \"{}\"", plugin->name());

		m_plugins.emplace_back(plugin);

		switch(plugin->managerKind()){
			case ManagerKind::render:{
				m_renderPlugins.emplace_back(plugin);
				break;
			}

			case ManagerKind::physics:{
				m_physicsPlugins.emplace_back(plugin);
				break;
			}

			case ManagerKind::input:{
				m_inputPlugins.emplace_back(plugin);
				break;
			}

			case ManagerKind::app:{
				m_appPlugins.emplace_back(plugin);
				break;
			}

			default: break;
		}
	}
}

std::atomic_flag sys::Manager::m_initFlag = ATOMIC_FLAG_INIT;

int sys::Manager::exec(PresentFn presentFn){
	Ticker ticker;

	if(!m_running){
		init();
	}

	while(m_running){
		auto dt = ticker.tick();
		update(dt);
		presentFn();
	}

	m_appManager.reset();
	m_renderManager.reset();
	m_physicsManager.reset();
	m_inputManager.reset();

	gpweSysManager = this;

	return 0;
}

void sys::Manager::update(float dt){
	m_inputManager->update(dt);
	m_appManager->update(dt);
	m_physicsManager->update(dt);
	m_renderManager->update(dt);
	m_renderManager->present(&gpweCamera);
}

void sys::Manager::setLogManager(Ptr<LogManager> manager){
	m_logManager = std::move(manager);
}

void sys::Manager::setRenderManager(Ptr<RenderManager> manager){
	m_renderManager = std::move(manager);
}

void sys::Manager::setPhysicsManager(Ptr<PhysicsManager> manager){
	m_physicsManager = std::move(manager);
}

void sys::Manager::setInputManager(Ptr<InputManager> manager){
	m_inputManager = std::move(manager);
}

void sys::Manager::setAppManager(Ptr<AppManager> manager){
	m_appManager = std::move(manager);
}

void sys::Manager::exit(){
	m_running = false;
}

void sys::Manager::init(){
	if(m_initFlag.test_and_set()){
		log::outLn("sys::Manager::init() called more than once");
		return;
	}

	gpweSysManager = this;

	//auto randNode = FastNoise::New<FastNoise::Value>(FastSIMD::Level_Null);

	auto launchT = std::chrono::system_clock::now();
	std::time_t launchTime = std::chrono::system_clock::to_time_t(launchT);
	log::outLn("{}", std::ctime(&launchTime));

	log::out("{:^{}}\n\n",
		format(
			"-- General Purpose World Engine v{}.{}.{}/{} --",
			GPWE_VERSION_MAJOR, GPWE_VERSION_MINOR, GPWE_VERSION_PATCH, GPWE_VERSION_GIT
		),
		30
	);

	initBaseLibraries();
	loadPlugins();

	auto ensureManager = [this](StrView kind_, auto &&manager, auto &&plugins){
		if(manager) return;

		if(plugins.empty()){
			log::outLn(log::Kind::error, "No {} plugins found", kind_);
			std::exit(4);
		}

		auto base = plugins[0]->createManager();
		manager = UniquePtr(reinterpret_cast<decltype(manager.get())>(base.release()));
	};

	ensureManager("render", m_renderManager, m_renderPlugins);
	ensureManager("physics", m_physicsManager, m_physicsPlugins);
	ensureManager("input", m_inputManager, m_inputPlugins);
	ensureManager("app", m_appManager, m_appPlugins);

	m_renderManager->setArg(m_renderArg);
	m_renderManager->setRenderSize(m_rW, m_rH);

	// TODO: create, initialize, update and destroy managers on separate threads

	/*
	m_threads[(std::size_t)ThreadIdx::app]
		= Thread([this]{ threadFn(m_appManager, m_workQueues[(std::size_t)ThreadIdx::app]); });
	*/

	if(m_logManager) m_logManager->init();
	m_inputManager->init();
	m_renderManager->init();
	m_physicsManager->init();
	m_appManager->init();

	m_running = true;
}

void sys::Manager::setRenderSize(std::uint16_t w, std::uint16_t h){
	m_rW = w;
	m_rH = h;
	if(m_renderManager){
		m_renderManager->setRenderSize(w, h);
	}
}

void *sys::alloc(std::size_t n){
	return std::malloc(n);
}

void sys::free(void *ptr){
	std::free(ptr);
}

void sys::setManager(SysManager *manager) noexcept{
	gpweSysManager = manager;
}

void sys::update(float dt){
	gpweSysManager->update(dt);
}

void sys::exit(){
	gpweSysManager->exit();
}

Camera *sys::camera() noexcept{ return &gpweCamera; }

sys::Manager *sys::manager() noexcept{ return gpweSysManager; }

app::Manager *sys::appManager() noexcept{ return gpweSysManager->appManager(); }

render::Manager *sys::renderManager() noexcept{ return gpweSysManager->renderManager(); }

physics::Manager *sys::physicsManager() noexcept{ return gpweSysManager->physicsManager(); }

log::Manager *sys::logManager() noexcept{
	if(!gpweSysManager) return &gpweDefaultLogManager;

	auto ret = gpweSysManager->logManager();
	return ret ? ret : &gpweDefaultLogManager;
}

input::Manager *sys::inputManager() noexcept{ return gpweSysManager->inputManager(); }

resource::Manager *sys::resourceManager() noexcept{ return &gpweResourceManager; }

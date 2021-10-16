#include <atomic>
#include <optional>
#include <functional>
#include <chrono>
#include <filesystem>

namespace fs = std::filesystem;

using GPWEProc = void(*)();

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
using LibHandle = HMODULE;
LibHandle loadLibrary(const char *path){ return LoadLibraryA(path); }
void closeLibrary(LibHandle lib){ FreeLibrary(lib); }
const char *loadLibraryError(){
	static Str errMsg;
	errMsg = std::to_string(GetLastError());
	return errMsg.c_str();
}
GPWEProc loadFunction(LibHandle lib, const char *name){
	return reinterpret_cast<GPWEProc>(GetProcAddress(lib, name));
}
#define LIB_PREFIX
#define LIB_EXT ".dll"
// LoadLibrary/FreeLibrary
#else
#include <dlfcn.h>
using LibHandle = void*;
LibHandle loadLibrary(const char *path){ return dlopen(path, RTLD_LAZY); }
void closeLibrary(LibHandle lib){ dlclose(lib); }
const char *loadLibraryError(){ return dlerror(); }
GPWEProc loadFunction(LibHandle lib, const char *name){
	return reinterpret_cast<GPWEProc>(dlsym(lib, name));
}
#define LIB_PREFIX "lib"
#define LIB_EXT ".so"
#endif

#include "gpwe/config.hpp"
#include "gpwe/log.hpp"
#include "gpwe/input.hpp"
#include "gpwe/sys.hpp"
#include "gpwe/render.hpp"
#include "gpwe/physics.hpp"
#include "gpwe/app.hpp"
#include "gpwe/resource.hpp"
#include "gpwe/Ticker.hpp"
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


static void *gpweInputLib = nullptr;
static void *gpweRendererLib = nullptr;
static void *gpwePhysicsLib = nullptr;
static void *gpweAppLib = nullptr;
static void *gpweRendererArg = nullptr;

sys::CreateManagerFn<sys::InputManager> gpweCreateInputManager = nullptr;
sys::CreateManagerFn<sys::AppManager> gpweCreateAppManager = nullptr;
sys::CreateManagerFn<sys::RenderManager> gpweCreateRenderManager = nullptr;
sys::CreateManagerFn<sys::PhysicsManager> gpweCreatePhysicsManager = nullptr;

UniquePtr<render::Manager> gpweRenderManager;
UniquePtr<physics::Manager> gpwePhysicsManager;
UniquePtr<input::Manager> gpweInputManager;
UniquePtr<app::Manager> gpweAppManager;

resource::Manager gpweResourceManager; // special, not a plugin

gpwe::Camera gpweCamera(90.f, 1290.f/720.f);

std::uint16_t gpweWidth, gpweHeight;

std::atomic_bool gpweRunning = false;

inline void logHeader(const Str &str){ gpwe::log("{:^30}\n\n", str); }

static void initLibraries(int argc, char *argv[]){
	StrView names[] = {
		"PhysFS",
		"libmagic",
		"Freetype",
		"FreeImage"
	};

	std::function<std::optional<Str>()> fns[] = {
		[argv]() -> std::optional<Str>{
			if(!PHYSFS_init(argv[0])){
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

		gpwe::log("{:<15}", fmt::format("{}...", name));

		auto res = fn();
		if(res){
			gpwe::log("Error");
			if(!res.value().empty()){
				gpwe::log(": {}\n", res.value());
			}

			std::exit(1);
		}
		else{
			gpwe::log("Done\n");
		}
	}

	gpwe::log("\n");
}

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

	gpwe::log("┏{:━^{}}┯{:━^{}}┓\n", "┫ API ┣", apiNameColWidth + 2, "┫ Version ┣", apiVersionColWidth + 2);

	for(std::size_t i = 0; i < std::size(libs); i++){
		auto name = libs[i];
		auto &&version = versions[i];

		gpwe::log("┃ {:<{}} │ {:<{}} ┃\n", name, apiNameColWidth, version, apiVersionColWidth);
	}

	gpwe::log("┗{:━<{}}┷{:━<{}}┛\n\n", "", apiNameColWidth + 2, "", apiVersionColWidth + 2);
}

void sys::setRendererArg(void *val){
	gpweRendererArg = val;
}

void sys::setRenderManager(UniquePtr<render::Manager> manager){
	gpweRenderManager = std::move(manager);
}

void sys::setPhysicsManager(UniquePtr<PhysicsManager> manager){
	gpwePhysicsManager = std::move(manager);
}

void sys::setInputManager(UniquePtr<InputManager> manager){
	gpweInputManager = std::move(manager);
}

void sys::setAppManager(UniquePtr<AppManager> manager){
	gpweAppManager = std::move(manager);
}

void sys::initSys(int argc, char *argv[]){
	auto launchT = std::chrono::system_clock::now();
	std::time_t launchTime = std::chrono::system_clock::to_time_t(launchT);
	logLn("{}", std::ctime(&launchTime));

	log("{:^{}}\n\n",
		format("-- General Purpose World Engine v{}.{}.{}/{} --", GPWE_VERSION_MAJOR, GPWE_VERSION_MINOR, GPWE_VERSION_PATCH, GPWE_VERSION_GIT),
		30
	);

	initLibraries(argc, argv);

	auto libIter = fs::directory_iterator();

	auto curPath = fs::current_path();

	std::vector<Str> appPaths, rendererPaths, inputPaths;

	for(auto it = fs::directory_iterator(curPath); it != fs::directory_iterator(); ++it){
		auto fileName = it->path().filename().string();

		constexpr StrView appPrefix = LIB_PREFIX "gpwe-app-";
		constexpr StrView rendererPrefix = LIB_PREFIX "gpwe-renderer-";
		constexpr StrView inputPrefix = LIB_PREFIX "gpwe-input-";

		if(fileName.find(appPrefix) == 0){
			if(fileName.rfind(LIB_EXT) != fileName.size() - (std::size(LIB_PREFIX) - 1)){
				continue;
			}

			auto appName = fileName.substr(appPrefix.size());
			if(appName == LIB_EXT){
				logErrorLn("Invalid app library '{}'", fileName);
				continue;
			}

			appPaths.emplace_back("./" + fileName);
		}
		else if(fileName.find(rendererPrefix) == 0){
			if(fileName.rfind(LIB_EXT) != fileName.size() - (std::size(LIB_PREFIX) - 1)){
				continue;
			}

			auto rendererName = fileName.substr(rendererPrefix.size());
			if(rendererName == LIB_EXT){
				logErrorLn("Invalid renderer library '{}'", fileName);
				continue;
			}

			rendererPaths.emplace_back("./" + fileName);
		}
		else if(fileName.find(inputPrefix) == 0){
			if(fileName.rfind(LIB_EXT) != fileName.size() - (std::size(LIB_PREFIX) - 1)){
				continue;
			}

			auto inputName = fileName.substr(inputPrefix.size());
			if(inputName == LIB_EXT){
				logErrorLn("Invalid input library '{}'", fileName);
				continue;
			}

			inputPaths.emplace_back("./" + fileName);
		}
	}

	if(!gpweInputManager && !gpweCreateInputManager){
		// Load input
		gpweInputLib = loadLibrary(!rendererPaths.empty() ? rendererPaths[0].c_str() : nullptr);
		if(!gpweInputLib){
			logErrorLn("{}", loadLibraryError());
			std::exit(3);
		}

		std::atexit([]{ closeLibrary(gpweInputLib); });

		auto createInputManagerFn = loadFunction(gpweInputLib, "gpweCreateInputManager");
		if(!createInputManagerFn){
			logErrorLn("{}", loadLibraryError());
			std::exit(3);
		}

		gpweCreateInputManager = reinterpret_cast<sys::CreateManagerFn<InputManager>>(createInputManagerFn);
	}

	if(!gpweRenderManager && !gpweCreateRenderManager){
		// Load renderer
		gpweRendererLib = loadLibrary(!rendererPaths.empty() ? rendererPaths[0].c_str() : nullptr);
		if(!gpweRendererLib){
			logErrorLn("{}", loadLibraryError());
			std::exit(3);
		}

		std::atexit([]{ closeLibrary(gpweRendererLib); });

		auto createRenderManagerFn = loadFunction(gpweRendererLib, "gpweCreateRenderManager");
		if(!createRenderManagerFn){
			logErrorLn("{}", loadLibraryError());
			std::exit(3);
		}

		gpweCreateRenderManager = reinterpret_cast<sys::CreateManagerFn<RenderManager>>(createRenderManagerFn);
	}

	if(!gpweAppManager && !gpweCreateAppManager){
		// Load app
		gpweAppLib = loadLibrary(!appPaths.empty() ? appPaths[0].c_str() : nullptr);
		if(!gpweAppLib){
			logErrorLn("{}", loadLibraryError());
			std::exit(3);
		}

		std::atexit([]{ closeLibrary(gpweAppLib); });

		auto createAppManagerFn = loadFunction(gpweAppLib, "gpweCreateAppManager");
		if(!createAppManagerFn){
			logErrorLn("{}", loadLibraryError());
			std::exit(3);
		}

		gpweCreateAppManager = reinterpret_cast<sys::CreateManagerFn<AppManager>>(createAppManagerFn);
	}

	printVersions();
}

void sys::initInput(){
	if(!gpweInputManager && !gpweCreateInputManager){
		logErrorLn("No input loaded");
		std::exit(4);
	}

	if(!gpweInputManager){
		gpweInputManager = gpweCreateInputManager();
		if(!gpweInputManager){
			logErrorLn("Error in gpweCreateInputManager");
			std::exit(4);
		}
	}

	gpweInputManager->init();
}

void sys::initRenderer(std::uint16_t w, std::uint16_t h){
	if(!gpweRenderManager && !gpweCreateRenderManager){
		logErrorLn("No renderer loaded");
		std::exit(4);
	}

	gpweWidth = w;
	gpweHeight = h;

	if(!gpweRenderManager){
		gpweRenderManager = gpweCreateRenderManager();
		if(!gpweRenderManager){
			logErrorLn("Error in gpweCreateRenderManager");
			std::exit(4);
		}
	}

	gpweRenderManager->setArg(gpweRendererArg);
	gpweRenderManager->init();
}

void sys::initApp(){
	if(!gpweAppManager && !gpweCreateAppManager){
		logErrorLn("No app loaded");
		std::exit(4);
	}

	if(!gpweAppManager){
		gpweAppManager = gpweCreateAppManager();
		if(!gpweAppManager){
			logErrorLn("Error in gpweCreateAppManager");
			std::exit(4);
		}
	}

	gpweAppManager->init();
}

void sys::tick(float dt){
	gpweInputManager->pumpEvents();
	gpweAppManager->update(dt);
	gpweRenderManager->present(&gpweCamera);
}

int sys::exec(PresentFn presentFn){
	std::fflush(stdout);

	Ticker ticker;

	gpweRunning = true;

	while(gpweRunning){
		tick(ticker.tick());
		presentFn();
	}

	// must destroy app before renderer
	gpweAppManager.reset();

	// Must destroy renderer before context and window
	gpweRenderManager.reset();

	gpweInputManager.reset();

	return 0;
}

void sys::exit(){
	gpweRunning = false;
}

void *sys::alloc(std::size_t n){
	return std::malloc(n);
}

void sys::free(void *ptr){
	std::free(ptr);
}

Camera *sys::camera(){ return &gpweCamera; }

app::Manager *sys::appManager(){ return gpweAppManager.get(); }

render::Manager *sys::renderManager(){ return gpweRenderManager.get(); }

input::Manager *sys::inputManager(){ return gpweInputManager.get(); }

resource::Manager *sys::resourceManager(){ return &gpweResourceManager; }

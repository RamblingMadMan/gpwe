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
LibHandle loadLibrary(const char *path){ return dlopen(path, RTLD_NOW); }
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
#include "gpwe/resource.hpp"
#include "gpwe/Ticker.hpp"
#include "gpwe/Camera.hpp"
#include "gpwe/Renderer.hpp"
#include "gpwe/App.hpp"

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

static void *gpweAppLib = nullptr;
static void *gpweRendererLib = nullptr;
static void *gpweRendererArg = nullptr;

using GPWECreateAppFn = UniquePtr<App>(*)();
using GPWECreateRendererFn = UniquePtr<Renderer>(*)(void*);

GPWECreateAppFn gpweCreateApp;
GPWECreateRendererFn gpweCreateRenderer;

input::Manager *gpweInputManager;
resource::Manager gpweResourceManager;

UniquePtr<App> gpweApp;
UniquePtr<Renderer> gpweRenderer;
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

void sys::initSys(
	int argc, char *argv[],
	input::Manager *inputManager_
){
	auto launchT = std::chrono::system_clock::now();
	std::time_t launchTime = std::chrono::system_clock::to_time_t(launchT);
	logLn("{}", std::ctime(&launchTime));

	log("{:^{}}\n\n",
		format("-- General Purpose World Engine v{}.{}.{}/{} --", GPWE_VERSION_MAJOR, GPWE_VERSION_MINOR, GPWE_VERSION_PATCH, GPWE_VERSION_GIT),
		30
	);

	if(!inputManager_){
		logErrorLn("No input manager given");
		std::exit(1);
	}

	gpweInputManager = inputManager_;

	initLibraries(argc, argv);

	auto libIter = fs::directory_iterator();

	auto curPath = fs::current_path();

	std::vector<Str> appPaths, rendererPaths;

	for(auto it = fs::directory_iterator(curPath); it != fs::directory_iterator(); ++it){
		auto fileName = it->path().filename().string();

		constexpr StrView appPrefix = LIB_PREFIX "gpwe-app-";
		constexpr StrView rendererPrefix = LIB_PREFIX "gpwe-renderer-";

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
	}

	// Load renderer
	gpweRendererLib = loadLibrary(!rendererPaths.empty() ? rendererPaths[0].c_str() : nullptr);
	if(!gpweRendererLib){
		logErrorLn("{}", loadLibraryError());
		std::exit(3);
	}

	std::atexit([]{ closeLibrary(gpweRendererLib); });

	// Load app
	gpweAppLib = loadLibrary(!appPaths.empty() ? appPaths[0].c_str() : nullptr);
	if(!gpweAppLib){
		logErrorLn("{}", loadLibraryError());
		std::exit(3);
	}

	std::atexit([]{ closeLibrary(gpweAppLib); });

	auto createRendererFn = loadFunction(gpweRendererLib, "gpweCreateRenderer");
	if(!createRendererFn){
		logErrorLn("{}", loadLibraryError());
		std::exit(3);
	}

	gpweCreateRenderer = reinterpret_cast<GPWECreateRendererFn>(createRendererFn);

	auto createAppFn = loadFunction(gpweAppLib, "gpweCreateApp");
	if(!createAppFn){
		logErrorLn("{}", loadLibraryError());
		std::exit(3);
	}

	gpweCreateApp = reinterpret_cast<GPWECreateAppFn>(createAppFn);

	printVersions();
}

void sys::initRenderer(std::uint16_t w, std::uint16_t h){
	gpweWidth = w;
	gpweHeight = h;

	gpweRenderer = gpweCreateRenderer(gpweRendererArg);

	std::fflush(stdout);

	if(!gpweRenderer){
		logErrorLn("Error in gpweCreateRenderer");
		std::exit(4);
	}
}

void sys::initApp(){
	gpweApp = gpweCreateApp();

	std::fflush(stdout);

	if(!gpweApp){
		gpweRenderer.reset();
		logErrorLn("Error in gpweCreateApp");
		std::exit(4);
	}
}

void sys::tick(float dt){
	gpweInputManager->pumpEvents();
	gpweApp->update(dt);
	gpweRenderer->present(&gpweCamera);
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
	gpweApp.reset();

	// Must destroy renderer before context and window
	gpweRenderer.reset();

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

Renderer *sys::renderer(){ return gpweRenderer.get(); }

input::Manager *sys::inputManager(){ return gpweInputManager; }

resource::Manager *sys::resourceManager(){ return &gpweResourceManager; }

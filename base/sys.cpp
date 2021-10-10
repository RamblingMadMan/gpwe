#include <string_view>
#include <atomic>
#include <optional>
#include <functional>
#include <memory>
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
	static std::string errMsg;
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
#include "gpwe/Camera.hpp"
#include "gpwe/Renderer.hpp"
#include "gpwe/App.hpp"

#include "physfs.h"

#include "ft2build.h"
#include FT_FREETYPE_H

#include "FreeImage.h"

#include "glm/glm.hpp"

#include "assimp/version.h"

using namespace gpwe;

static FT_Library gpweFtLib = nullptr;

void *gpweAppLib = nullptr;
void *gpweRendererLib = nullptr;

using GPWEProc = void(*)();
using GPWEGetProcFn = GPWEProc(*)(const char*);

using GPWECreateAppFn = std::unique_ptr<App>(*)();
using GPWECreateRendererFn = std::unique_ptr<Renderer>(*)(void*);

GPWECreateAppFn gpweCreateApp;
GPWECreateRendererFn gpweCreateRenderer;

input::Manager *gpweInputManager;
std::unique_ptr<App> gpweApp;
std::unique_ptr<Renderer> gpweRenderer;
gpwe::Camera gpweCamera(90.f, 1290.f/720.f);

std::uint16_t gpweWidth, gpweHeight;

std::atomic_bool gpweRunning = false;

inline void logHeader(const std::string &str){ gpwe::log("{:^30}\n\n", str); }

void initLibraries(int argc, char *argv[]){
	std::string_view names[] = {
		"PhysFS",
		"Freetype",
		"FreeImage"
	};

	std::function<std::optional<std::string>()> fns[] = {
		[argv]() -> std::optional<std::string>{
			if(!PHYSFS_init(argv[0])){
				return PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
			}

			return std::nullopt;
		},
		[]() -> std::optional<std::string>{
			auto err = FT_Init_FreeType(&gpweFtLib);
			if(err != FT_Err_Ok){
				// TODO: get error message
				return "";
			}

			std::atexit([]{ FT_Done_FreeType(gpweFtLib); });
			return std::nullopt;
		},
		[]() -> std::optional<std::string>{
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

void printVersions(){
	FT_Int ftMaj, ftMin, ftPatch;
	FT_Library_Version(gpweFtLib, &ftMaj, &ftMin, &ftPatch);

	std::string_view libs[] = {
		"PhysFS",
		"Freetype",
		"FreeImage",
		"Assimp"
	};

	PHYSFS_Version pfsVer;
	PHYSFS_getLinkedVersion(&pfsVer);

	std::string versions[] = {
		fmt::format("{}.{}.{}", pfsVer.major, pfsVer.minor, pfsVer.patch),
		fmt::format("{}.{}.{}", ftMaj, ftMin, ftPatch),
		FreeImage_GetVersion(),
		fmt::format("{}.{}.{}", aiGetVersionMajor(), aiGetVersionMinor(), aiGetVersionPatch())
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

void sys::init(
	int argc, char *argv[],
	input::Manager *inputManager_, std::uint16_t w, std::uint16_t h
){
	auto launchT = std::chrono::system_clock::now();
	std::time_t launchTime = std::chrono::system_clock::to_time_t(launchT);
	logLn("{}", std::ctime(&launchTime));

	log("{:^{}}\n\n",
		fmt::format("-- General Purpose World Engine v{}.{}.{}/{} --", GPWE_VERSION_MAJOR, GPWE_VERSION_MINOR, GPWE_VERSION_PATCH, GPWE_VERSION_GIT),
		30
	);

	if(!inputManager_){
		logErrorLn("No input manager given");
		std::exit(1);
	}

	gpweInputManager = inputManager_;
	gpweWidth = w;
	gpweHeight = h;

	initLibraries(argc, argv);

	auto libIter = fs::directory_iterator();

	auto curPath = fs::current_path();

	std::vector<std::string> appPaths, rendererPaths;

	for(auto it = fs::directory_iterator(curPath); it != fs::directory_iterator(); ++it){
		auto fileName = it->path().filename().string();

		constexpr std::string_view appPrefix = LIB_PREFIX "gpwe-app-";
		constexpr std::string_view rendererPrefix = LIB_PREFIX "gpwe-renderer-";

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

	// Load app
	gpweAppLib = loadLibrary(appPaths[0].c_str());
	if(!gpweAppLib){
		logErrorLn("{}", loadLibraryError());
		std::exit(3);
	}

	std::atexit([]{ closeLibrary(gpweAppLib); });

	// Load renderer
	gpweRendererLib = loadLibrary(rendererPaths[0].c_str());
	if(!gpweRendererLib){
		logErrorLn("{}", loadLibraryError());
		std::exit(3);
	}

	std::atexit([]{ closeLibrary(gpweRendererLib); });

	auto createAppFn = loadFunction(gpweAppLib, "gpweCreateApp");
	if(!createAppFn){
		logErrorLn("{}", loadLibraryError());
		std::exit(3);
	}

	gpweCreateApp = reinterpret_cast<GPWECreateAppFn>(createAppFn);

	auto createRendererFn = loadFunction(gpweRendererLib, "gpweCreateRenderer");
	if(!createRendererFn){
		logErrorLn("{}", loadLibraryError());
		std::exit(3);
	}

	gpweCreateRenderer = reinterpret_cast<GPWECreateRendererFn>(createRendererFn);

	printVersions();
}

int sys::exec(PresentFn presentFn, void *rendererArg){
	std::fflush(stdout);

	logHeader("Starting Engine");

	gpweCamera = Camera(90.f, float(gpweWidth) / float(gpweHeight));

	gpweRenderer = gpweCreateRenderer(rendererArg);

	if(!gpweRenderer){
		logErrorLn("Error in gpweCreateRenderer");
		std::exit(4);
	}

	gpweApp = gpweCreateApp();

	if(!gpweApp){
		gpweRenderer.reset();
		logErrorLn("Error in gpweCreateApp");
		std::exit(4);
	}

	std::fflush(stdout);

	using Clock = std::chrono::high_resolution_clock;
	using Seconds = std::chrono::duration<float>;

	auto startT = Clock::now();
	auto loopT = startT;

	gpweRunning = true;

	while(gpweRunning){
		auto loopEndT = Clock::now();
		auto loopDt = Seconds(loopEndT - loopT).count();

		gpweInputManager->pumpEvents();

		gpweApp->update(loopDt);

		gpweRenderer->present(&gpweCamera);

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

Camera *sys::camera(){ return &gpweCamera; }

Renderer *sys::renderer(){ return gpweRenderer.get(); }

input::Manager *sys::inputManager(){ return gpweInputManager; }

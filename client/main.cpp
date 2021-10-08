#include <functional>
#include <optional>
#include <string_view>
#include <memory>

#include "SDL.h"

#include "ft2build.h"
#include FT_FREETYPE_H

#include "FreeImage.h"

#include "gpwe/config.hpp"
#include "gpwe/sys.hpp"
#include "gpwe/log.hpp"
#include "gpwe/Renderer.hpp"

using GLProc = void(*)();
using GLGetProcFn = GLProc(*)(const char*);

SDL_Window *gpweWindow = nullptr;
SDL_GLContext gpweContext = nullptr;
void *gpweRendererLib = nullptr;
std::unique_ptr<gpwe::Renderer> gpweRenderer;
std::unique_ptr<gpwe::Renderer>(*gpweCreateRendererFn)(GLGetProcFn);

static FT_Library gpweFtLib = nullptr;

constexpr int apiNameColWidth = 16;
constexpr int apiVersionColWidth = 30;
constexpr int apiVersionWidth = apiNameColWidth + apiVersionColWidth;

inline void logHeader(const std::string &str){ gpwe::log("{:^30}\n\n", str); }

void initLibraries(){
	std::string_view names[] = {
		"SDL",
		"Freetype",
		"FreeImage"
	};

	std::function<std::optional<std::string>()> fns[] = {
		[]() -> std::optional<std::string>{
			if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0){
				return SDL_GetError();
			}

			std::atexit(SDL_Quit);
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

void initVideo(){
	constexpr std::string_view msgs[] = {
		"Setting Attributes",
		"Creating window",
		"Creating context",
		"Loading renderer"
	};

	std::function<std::optional<std::string>()> fns[] = {
		[]() -> std::optional<std::string>{
			constexpr std::pair<SDL_GLattr, int> ps[] = {
				{ SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE },
				{ SDL_GL_CONTEXT_MAJOR_VERSION, 4 },
				{ SDL_GL_CONTEXT_MINOR_VERSION, 3 },
				{ SDL_GL_RED_SIZE, 8 },
				{ SDL_GL_GREEN_SIZE, 8 },
				{ SDL_GL_BLUE_SIZE, 8 },
				{ SDL_GL_ALPHA_SIZE, 8 },
				{ SDL_GL_DEPTH_SIZE, 16 }
			};

			for(std::size_t i = 0; i < std::size(ps); i++){
				auto &&p = ps[i];
				if(SDL_GL_SetAttribute(p.first, p.second) != 0){
					return SDL_GetError();
				}
			};

			return std::nullopt;
		},
		[]() -> std::optional<std::string>{
			gpweWindow = SDL_CreateWindow("GPWE", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_OPENGL);
			if(!gpweWindow){
				return SDL_GetError();
			}

			std::atexit([]{ SDL_DestroyWindow(gpweWindow); });

			return std::nullopt;
		},
		[]() -> std::optional<std::string>{
			gpweContext = SDL_GL_CreateContext(gpweWindow);
			if(!gpweContext){
				return SDL_GetError();
			}

			std::atexit([]{ SDL_GL_DeleteContext(gpweContext); });

			SDL_GL_MakeCurrent(gpweWindow, gpweContext);
			if(SDL_GL_SetSwapInterval(-1) != 0){
				SDL_GL_SetSwapInterval(1);
			}

			return std::nullopt;
		},
		[]() -> std::optional<std::string>{
			gpweRendererLib = SDL_LoadObject("./libgpwe-renderer-gl43.so");
			if(!gpweRendererLib){
				return SDL_GetError();
			}

			std::atexit([]{ SDL_UnloadObject(gpweRendererLib); });

			gpweCreateRendererFn = reinterpret_cast<std::unique_ptr<gpwe::Renderer>(*)(GLGetProcFn)>(SDL_LoadFunction(gpweRendererLib, "gpweCreateRenderer_gl43"));
			if(!gpweCreateRendererFn){
				return SDL_GetError();
			}

			return std::nullopt;
		}
	};

	logHeader("Initializing Video");

	for(std::size_t i = 0; i < std::size(msgs); i++){
		auto msg = msgs[i];

		gpwe::log("{:<24}", fmt::format("{}...", msg));

		auto res = fns[i]();
		if(res){
			if(!res.value().empty()){
				gpwe::log("Error: {}\n", res.value());
			}
			else{
				gpwe::log("Error\n");
			}

			std::exit(2);
		}

		gpwe::log("Done\n");
	}

	gpwe::log("\n");
}

void printVersions(){
	SDL_version sdlVer;
	SDL_GetVersion(&sdlVer);

	FT_Int ftMaj, ftMin, ftRev;
	FT_Library_Version(gpweFtLib, &ftMaj, &ftMin, &ftRev);

	std::string_view libs[] = {
		"SDL",
		"Freetype",
		"FreeImage"
	};

	std::string versions[] = {
		fmt::format("{}.{}.{}", sdlVer.major, sdlVer.minor, sdlVer.patch),
		fmt::format("{}.{}.{}", ftMaj, ftMin, ftRev),
		FreeImage_GetVersion()
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

void gpwe::sys::init(){
	gpwe::log("{:^{}}\n\n",
		fmt::format("-- General Purpose World Engine v{}.{}.{}/{} --", GPWE_VERSION_MAJOR, GPWE_VERSION_MINOR, GPWE_VERSION_PATCH, GPWE_VERSION_GIT),
		30
	);

	initLibraries();
	initVideo();

	printVersions();
}

int gpwe::sys::exec(){
	logHeader("Starting Engine");

	gpweRenderer = gpweCreateRendererFn([](const char *name) -> GLProc{ return (GLProc)SDL_GL_GetProcAddress(name); });

	bool running = true;

	SDL_Event ev;

	while(running){
		while(SDL_PollEvent(&ev)){
			if(ev.type == SDL_QUIT){
				running = false;
				break;
			}
		}

		gpweRenderer->present();

		SDL_GL_SwapWindow(gpweWindow);
	}

	// Must destroy renderer before context and window
	gpweRenderer.reset();

	return 0;
}

int main(int argc, char *argv[]){
	gpwe::sys::init();

	return gpwe::sys::exec();
}

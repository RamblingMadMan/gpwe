#include <ctime>
#include <functional>
#include <optional>
#include <string_view>
#include <memory>
#include <chrono>

#include "SDL.h"

#include "ft2build.h"
#include FT_FREETYPE_H

#include "FreeImage.h"

#include "glm/glm.hpp"

#include "gpwe/config.hpp"
#include "gpwe/sys.hpp"
#include "gpwe/log.hpp"
#include "gpwe/Camera.hpp"
#include "gpwe/Shape.hpp"
#include "gpwe/Renderer.hpp"

using namespace gpwe;

using GLProc = void(*)();
using GLGetProcFn = GLProc(*)(const char*);

SDL_Window *gpweWindow = nullptr;
SDL_GLContext gpweContext = nullptr;
void *gpweRendererLib = nullptr;
std::unique_ptr<Renderer> gpweRenderer;
std::unique_ptr<Renderer>(*gpweCreateRendererFn)(GLGetProcFn);

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
			#ifndef NDEBUG
				{ SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG },
			#endif
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

			SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");

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

void sys::init(){
	auto launchT = std::chrono::system_clock::now();
	std::time_t launchTime = std::chrono::system_clock::to_time_t(launchT);
	logLn("{}", std::ctime(&launchTime));

	log("{:^{}}\n\n",
		fmt::format("-- General Purpose World Engine v{}.{}.{}/{} --", GPWE_VERSION_MAJOR, GPWE_VERSION_MINOR, GPWE_VERSION_PATCH, GPWE_VERSION_GIT),
		30
	);

	initLibraries();
	initVideo();

	printVersions();
}

int sys::exec(){
	std::fflush(stdout);

	logHeader("Starting Engine");

	gpweRenderer = gpweCreateRendererFn([](const char *name) -> GLProc{ return (GLProc)SDL_GL_GetProcAddress(name); });

	Camera cam(90.f, 1280.f / 720.f);

	cam.setPosition(glm::vec3(0.f, 0.f, -1.f));

	shapes::Cube testShape(0.5f);
	shapes::Square screenShape(2.f);

	auto testGroup = gpweRenderer->createGroup(&testShape);
	auto screenGroup = gpweRenderer->createGroup(&screenShape);

	std::fflush(stdout);

	SDL_Event ev;

	using Clock = std::chrono::high_resolution_clock;
	using Seconds = std::chrono::duration<float>;

	auto startT = Clock::now();
	auto loopT = startT;

	bool running = true;
	bool rotateCam = false;

	float camSpeed = 0.1f;
	glm::vec2 camRot;
	glm::vec3 camMove = { 0.f, 0.f, 0.f };

	while(running){
		auto loopEndT = Clock::now();
		auto loopDt = Seconds(loopEndT - loopT).count();

		camRot = { 0.f, 0.f };

		while(SDL_PollEvent(&ev)){
			if(ev.type == SDL_QUIT){
				running = false;
				break;
			}

			switch(ev.type){
				case SDL_QUIT:{
					running = false;
					break;
				}

				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:{
					bool pressed = ev.type == SDL_MOUSEBUTTONDOWN;
					if(ev.button.button == SDL_BUTTON_LEFT){
						rotateCam = pressed;
					}

					break;
				}

				case SDL_MOUSEMOTION:{
					if(rotateCam){
						camRot += glm::vec2(ev.motion.xrel, ev.motion.yrel);
					}

					break;
				}

				case SDL_KEYDOWN:
				case SDL_KEYUP:{
					if(ev.key.repeat) break;

					bool pressed = ev.type == SDL_KEYDOWN;
					switch(ev.key.keysym.sym){
						case SDLK_w:
						case SDLK_s:{
							bool forward = ev.key.keysym.sym == SDLK_w;
							const glm::vec3 dir = { 0.f, 0.f, forward ? 1.f : -1.f };
							camMove += pressed ? dir : -dir;
							break;
						}

						case SDLK_a:
						case SDLK_d:{
							bool right = ev.key.keysym.sym == SDLK_d;
							const glm::vec3 dir = { right ? 1.f : -1.f, 0.f, 0.f };
							camMove += pressed ? dir : -dir;
							break;
						}

						case SDLK_SPACE:
						case SDLK_LCTRL:{
							bool up = ev.key.keysym.sym == SDLK_SPACE;
							const glm::vec3 dir = { 0.f, up ? 1.f : -1.f, 0.f };
							camMove += pressed ? dir : -dir;
							break;
						}

						case SDLK_ESCAPE:{
							running = false;
							break;
						}

						default: break;
					}

					break;
				}

				default: break;
			}
		}

		auto moveForward = camMove.z * cam.forward();
		auto moveRight = camMove.x * cam.right();
		auto moveUp = glm::vec3(0.f, camMove.y, 0.f);

		auto moveAmnt = moveForward + moveRight + moveUp;

		if(glm::length(moveAmnt) > 0.f){
			cam.translate(glm::normalize(moveAmnt) * loopDt * 0.001f);
		}

		if(rotateCam){
			camRot *= camSpeed * loopDt;
			cam.rotate(glm::radians(-camRot.x), glm::vec3(0.f, 1.f, 0.f));
			cam.rotate(glm::radians(camRot.y), glm::vec3(1.f, 0.f, 0.f));
		}

		gpweRenderer->present(&cam);

		SDL_GL_SwapWindow(gpweWindow);
	}

	// These aren't required
	gpweRenderer->destroyGroup(testGroup);
	gpweRenderer->destroyGroup(screenGroup);

	// Must destroy renderer before context and window
	gpweRenderer.reset();

	return 0;
}

int main(int argc, char *argv[]){
	sys::init();
	return sys::exec();
}

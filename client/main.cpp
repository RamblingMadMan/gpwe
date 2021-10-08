#include <functional>
#include <optional>
#include <string_view>
#include <list>

#include "SDL.h"

#include "ft2build.h"
#include FT_FREETYPE_H

#include "FreeImage.h"

#include "gpwe/config.hpp"
#include "gpwe/sys.hpp"
#include "gpwe/log.hpp"

SDL_Window *gpweWindow = nullptr;

static FT_Library gpweFtLib;

constexpr int apiNameColWidth = 16;
constexpr int apiVersionColWidth = 30;
constexpr int apiVersionWidth = apiNameColWidth + apiVersionColWidth;

namespace {
	std::vector<std::function<void()>> exitFns;

	template<typename Fn>
	void atExit(Fn &&fn){
		exitFns.emplace_back(std::forward<Fn>(fn));
	}
}

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

	gpwe::log("{:^30}\n\n", "Initializing Libraries");

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

	gpwe::log("┏{:━^{}}┯{:━^{}}┓\n", "┫ Library ┣", apiNameColWidth + 2, "┫ Version ┣", apiVersionColWidth + 2);

	for(std::size_t i = 0; i < std::size(libs); i++){
		auto name = libs[i];
		auto &&version = versions[i];

		gpwe::log("┃ {:<{}} │ {:<{}} ┃\n", name, apiNameColWidth, version, apiVersionColWidth);
	}

	gpwe::log("┗{:━<{}}┷{:━<{}}┛\n", "", apiNameColWidth + 2, "", apiVersionColWidth + 2);
}

void gpwe::sys::init(){
	std::atexit([]{
		for(auto it = rbegin(exitFns); it != rend(exitFns); ++it){
			(*it)();
		}
	});

	gpwe::log("{:^{}}\n\n", "-- General Purpose World Engine v0.0.0 --", 30);

	initLibraries();
	printVersions();

	gpwe::log("{:<}", "Creating Window...");

	gpweWindow = SDL_CreateWindow("GPWE", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_OPENGL);
	if(!gpweWindow){
		gpwe::log("");
	}
}

int main(int argc, char *argv[]){
	gpwe::sys::init();

	return 0;
}

#include <ctime>
#include <functional>
#include <optional>
#include <string_view>
#include <memory>
#include <chrono>

#include "SDL.h"

#include "gpwe/config.hpp"
#include "gpwe/log.hpp"
#include "gpwe/sys.hpp"
#include "gpwe/input.hpp"
#include "gpwe/render.hpp"
#include "gpwe/app.hpp"
#include "gpwe/Camera.hpp"
#include "gpwe/Shape.hpp"

using namespace gpwe;

namespace {
	inline input::MouseButton sdlToMouseBtn(std::uint8_t btn){
		using Button = input::MouseButton;
		switch(btn){
			case SDL_BUTTON_LEFT: return Button::left;
			case SDL_BUTTON_RIGHT: return Button::right;
			case SDL_BUTTON_MIDDLE: return Button::middle;
			case SDL_BUTTON_X1: return Button::scrollDown;
			case SDL_BUTTON_X2: return Button::scrollUp;
			default: return Button::count;
		}
	}

	inline input::Key sdlkToKey(SDL_Keycode key){
		using Key = input::Key;
		switch(key){
#define CASETO(sdlk, val) case sdlk: return Key::val

			CASETO(SDLK_a, a);
			CASETO(SDLK_b, b);
			CASETO(SDLK_c, c);
			CASETO(SDLK_d, d);
			CASETO(SDLK_e, e);
			CASETO(SDLK_f, f);
			CASETO(SDLK_g, g);
			CASETO(SDLK_h, h);
			CASETO(SDLK_i, i);
			CASETO(SDLK_j, j);
			CASETO(SDLK_k, k);
			CASETO(SDLK_l, l);
			CASETO(SDLK_m, m);
			CASETO(SDLK_n, n);
			CASETO(SDLK_o, o);
			CASETO(SDLK_p, p);
			CASETO(SDLK_q, q);
			CASETO(SDLK_r, r);
			CASETO(SDLK_s, s);
			CASETO(SDLK_t, t);
			CASETO(SDLK_u, u);
			CASETO(SDLK_v, v);
			CASETO(SDLK_w, w);
			CASETO(SDLK_y, x);
			CASETO(SDLK_x, y);
			CASETO(SDLK_z, z);

			CASETO(SDLK_0, _0);
			CASETO(SDLK_1, _1);
			CASETO(SDLK_2, _2);
			CASETO(SDLK_3, _3);
			CASETO(SDLK_4, _4);
			CASETO(SDLK_5, _5);
			CASETO(SDLK_6, _6);
			CASETO(SDLK_7, _7);
			CASETO(SDLK_8, _8);
			CASETO(SDLK_9, _9);

			CASETO(SDLK_ESCAPE, escape);
			CASETO(SDLK_BACKQUOTE, tilde);
			CASETO(SDLK_TAB, tab);
			CASETO(SDLK_SPACE, space);

			CASETO(SDLK_LSHIFT, lshift);
			CASETO(SDLK_LCTRL, lctrl);
			CASETO(SDLK_LGUI, lsuper);
			CASETO(SDLK_LALT, lalt);

			CASETO(SDLK_RSHIFT, rshift);
			CASETO(SDLK_RCTRL, rctrl);
			CASETO(SDLK_RALT, ralt);

#undef CASETO
			default: return Key::count;
		}
	}
}

class SDLMouse: public input::Mouse{
	public:
		SDLMouse(Nat32 id_): Mouse(id_){}

		void setCapture(bool enabled) override{
			SDL_CaptureMouse(enabled ? SDL_TRUE : SDL_FALSE);
		}

		void setRelativeMode(bool enabled) override{
			SDL_SetRelativeMouseMode(enabled ? SDL_TRUE : SDL_FALSE);
		}
};

class SDLInputManager: public input::Manager{
	public:
		SDLInputManager(){
			create<input::Mouse>(0);
			create<input::Keyboard>(0);
		}

	protected:
		void pumpEvents() override{
			while(SDL_PollEvent(&ev)){
				switch(ev.type){
					case SDL_QUIT:{
						m_sys.exitEvent();
						break;
					}

					case SDL_KEYDOWN:
					case SDL_KEYUP:{
						if(ev.key.repeat) break;

						auto key = sdlkToKey(ev.key.keysym.sym);
						if(key == input::Key::count) break;

						for(auto &&kb : managed<input::Keyboard>()){
							kb->keyEvent().emit(key, ev.type == SDL_KEYDOWN);
						}

						break;
					}

					case SDL_MOUSEBUTTONDOWN:
					case SDL_MOUSEBUTTONUP:{
						auto btn = sdlToMouseBtn(ev.button.button);
						if(btn == input::MouseButton::count) break;

						for(auto &&mouse : managed<input::Mouse>()){
							mouse->buttonEvent().emit(btn, ev.type == SDL_MOUSEBUTTONDOWN);
						}



						break;
					}

					case SDL_MOUSEWHEEL:{
						if(ev.wheel.direction == SDL_MOUSEWHEEL_FLIPPED){
							ev.wheel.x *= -1;
							ev.wheel.y *= -1;
						}

						for(auto &&mouse : managed<input::Mouse>()){
							mouse->scrollEvent().emit(ev.wheel.x, ev.wheel.y);
						}

						break;
					}

					case SDL_MOUSEMOTION:{
						for(auto &&mouse : managed<input::Mouse>()){
							mouse->moveEvent().emit(ev.motion.xrel, ev.motion.yrel);
						}

						break;
					}

					default: break;
				}
			}
		}

		UniquePtr<input::Mouse> doCreateMouse(Nat32 id) override{
			return makeUnique<SDLMouse>(id);
		}

		SDL_Event ev;
};

SDL_Window *gpweWin = nullptr;
SDL_GLContext gpweCtx = nullptr;

int main(int argc, char *argv[]){
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0){
		log::errorLn("{}", SDL_GetError());
		return 1;
	}

	std::atexit(SDL_Quit);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#ifndef NDEBUG
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
	SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, SDL_TRUE);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);

	// TODO: set this for windowed only
	SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");

	gpweWin = SDL_CreateWindow("GPWE", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_OPENGL);
	if(!gpweWin){
		log::errorLn("{}", SDL_GetError());
		return 1;
	}

	std::atexit([]{ SDL_DestroyWindow(gpweWin); });

	gpweCtx = SDL_GL_CreateContext(gpweWin);
	if(!gpweCtx){
		log::errorLn("{}", SDL_GetError());
		return 1;
	}

	std::atexit([]{ SDL_GL_DeleteContext(gpweCtx); });

	if(SDL_GL_MakeCurrent(gpweWin, gpweCtx) != 0){
		log::errorLn("{}", SDL_GetError());
		return 1;
	}

	if(SDL_GL_SetSwapInterval(-1) != 0){
		SDL_GL_SetSwapInterval(1);
	}

	using Proc = void(*)();
	auto loadGLFn = +[](const char *name){ return (Proc)SDL_GL_GetProcAddress(name); };

	auto manager = makeUnique<sys::Manager>();

	manager->setInputManager(gpwe::makeUnique<SDLInputManager>());
	manager->setRenderArg((void*)loadGLFn);
	manager->setRenderSize(1280, 720);

	manager->setArgs(argc, argv);
	manager->init();

	return manager->exec(std::bind(SDL_GL_SwapWindow, gpweWin));
}

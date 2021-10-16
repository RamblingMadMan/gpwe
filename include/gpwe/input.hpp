#ifndef GPWE_INPUT_HPP
#define GPWE_INPUT_HPP 1

#include <cstdint>
#include <map>

#include "Manager.hpp"

namespace gpwe::input{
	class System{
		public:
			using ExitEventFn = std::function<void()>;

			using ExitEventIter = List<ExitEventFn>::iterator;

			template<typename Fn>
			ExitEventIter onExitEvent(Fn &&fn){
				return m_exitFns.emplace(m_exitFns.end(), std::forward<Fn>(fn));
			}

			void exitEvent(){
				for(auto &&fn : m_exitFns){
					fn();
				}
			}

		private:
			System() noexcept{}

			List<ExitEventFn> m_exitFns;

			friend class Manager;
	};

	enum class Key{
		a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z,
		_0, _1, _2, _3, _4, _5, _6, _7, _8, _9,

		escape,
		tab,
		return_,
		tilde,
		space,

		lshift,
		lctrl,
		lalt,
		lsuper,
		rshift,
		rctrl,
		ralt,

		kp0, kp1, kp2, kp3, kp4, kp5, kp6, kp7, kp8, kp9,
		kpMinus, kpPlus, kpDiv, kpMul, kpDot,
		kpEnter,

		count
	};

	class Keyboard{
		public:
			using KeyEventFn = std::function<void(Key, bool)>;

			using KeyEventIter = List<KeyEventFn>::iterator;

			explicit Keyboard(std::uint32_t id_) noexcept
				: m_id(id_){}

			template<typename Fn>
			KeyEventIter onKeyEvent(Fn &&fn){
				return m_keyFns.emplace(m_keyFns.end(), std::forward<Fn>(fn));
			}

			void keyEvent(Key key, bool pressed){
				for(auto &&fn : m_keyFns){
					fn(key, pressed);
				}
			}

		private:
			std::uint32_t m_id;
			List<KeyEventFn> m_keyFns;
	};

	enum class MouseButton: std::uint8_t{
		left, right, middle,
		scrollUp, scrollDown,
		scrollLeft, scrollRight,

		count
	};

	class Mouse{
		public:
			using ButtonEventFn = std::function<void(MouseButton, bool)>;
			using MoveEventFn = std::function<void(std::int32_t, std::int32_t)>;

			using ButtonEventIter = List<ButtonEventFn>::iterator;
			using MoveEventIter = List<MoveEventFn>::iterator;

			explicit Mouse(std::uint32_t id_) noexcept
				: m_id(id_){}

			template<typename Fn>
			ButtonEventIter onButtonEvent(Fn &&fn){
				return m_btnFns.emplace(m_btnFns.end(), std::forward<Fn>(fn));
			}

			template<typename Fn>
			MoveEventIter onMoveEvent(Fn &&fn){
				return m_moveFns.emplace(m_moveFns.end(), std::forward<Fn>(fn));
			}

			void buttonEvent(MouseButton btn, bool pressed){
				for(auto &&fn : m_btnFns){
					fn(btn, pressed);
				}
			}

			void moveEvent(std::int32_t xrel, std::int32_t yrel){
				for(auto &&fn : m_moveFns){
					fn(xrel, yrel);
				}
			}

		private:
			std::uint32_t m_id;
			List<ButtonEventFn> m_btnFns;
			List<MoveEventFn> m_moveFns;
	};

	class Gamepad{
		public:
			explicit Gamepad(std::uint32_t id_) noexcept
				: m_id(id_){}

			std::uint32_t id() const noexcept{ return m_id; }

		private:
			std::uint32_t m_id;
	};

	class Manager: public gpwe::Manager<Manager>{
		public:
			using PumpEventFn = std::function<void()>;

			using PumpEventIt = List<PumpEventFn>::iterator;

			virtual ~Manager() = default;

			void pumpEvents(){
				for(auto &&fn : m_pumpFns){
					fn();
				}

				doPumpEvents();
			}

			template<typename Fn>
			PumpEventIt onPumpEvents(Fn &&fn){
				return m_pumpFns.emplace(m_pumpFns.end(), std::forward<Fn>(fn));
			}

			Keyboard *createKeyboard(){ return &m_kbs.emplace_back(m_kbs.size()); }
			Mouse *createMouse(){ return &m_mice.emplace_back(m_mice.size()); }
			Gamepad *createGamepad(){ return &m_gamepads.emplace_back(m_gamepads.size()); }

			std::uint32_t numKeyboards() const noexcept{ return m_kbs.size(); }
			std::uint32_t numMice() const noexcept{ return m_mice.size(); }
			std::uint32_t numGamepads() const noexcept{ return m_gamepads.size(); }

			System *system() noexcept{ return &m_sys; }

			Keyboard *keyboard(std::uint32_t idx = 0) noexcept{
				return getFromList(m_kbs, idx);
			}

			Mouse *mouse(std::uint32_t idx = 0) noexcept{
				return getFromList(m_mice, idx);
			}

			Gamepad *gamepad(std::uint32_t idx = 0) noexcept{
				return getFromList(m_gamepads, idx);
			}

		protected:
			virtual void doPumpEvents(){}

			template<typename T>
			static T *getFromList(List<T> &l, std::uint32_t idx){
				if(idx >= l.size()) return nullptr;

				auto res = l.begin();
				while(idx > 0){
					++res;
					--idx;
				}

				return &*res;
			}

			System m_sys;
			List<Keyboard> m_kbs;
			List<Mouse> m_mice;
			List<Gamepad> m_gamepads;

			List<PumpEventFn> m_pumpFns;
	};
}

#endif // !GPWE_INPUT_HPP

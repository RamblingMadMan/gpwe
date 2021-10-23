#ifndef GPWE_INPUT_HPP
#define GPWE_INPUT_HPP 1

#include <cstdint>
#include <map>

#include "util/Event.hpp"
#include "Manager.hpp"

namespace gpwe::input{
	class System: public Object<System>{
		public:
			using ExitEventFn = Fn<void()>;

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

	enum class MouseButton: std::uint8_t{
		left, right, middle,
		scrollUp, scrollDown,
		scrollLeft, scrollRight,

		count
	};

	class Keyboard;
	class Mouse;
	class Gamepad;

	class Manager: public gpwe::Manager<Manager, ManagerKind::input, Mouse, Keyboard>{
		public:
			using PumpEventFn = Fn<void()>;

			using PumpEventIt = List<PumpEventFn>::iterator;

			virtual ~Manager() = default;

			void update(float dt) override{
				(void)dt;

				for(auto &&fn : m_pumpFns){
					fn();
				}

				pumpEvents();
			}

			template<typename Fn>
			PumpEventIt onPumpEvents(Fn &&fn){
				return m_pumpFns.emplace(m_pumpFns.end(), std::forward<Fn>(fn));
			}

			System *system() noexcept{ return &m_sys; }

		protected:
			virtual void pumpEvents(){}

			virtual UniquePtr<Keyboard> doCreateKeyboard(std::uint32_t id){ return makeUnique<Keyboard>(id); }
			virtual UniquePtr<Mouse> doCreateMouse(std::uint32_t id){ return makeUnique<Mouse>(id); }
			virtual UniquePtr<Gamepad> doCreateGamepad(std::uint32_t id){ return makeUnique<Gamepad>(id); }

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

			List<PumpEventFn> m_pumpFns;

			friend class Keyboard;
			friend class Mouse;
			friend class Gamepad;
	};

	class Keyboard: public Managed<Keyboard, &Manager::doCreateKeyboard>{
		public:
			using KeyEvent = Event<Key, bool>;
			using KeyEventFn = Fn<void(Key, bool)>;

			using KeyEventIter = List<KeyEventFn>::iterator;

			explicit Keyboard(std::uint32_t id_) noexcept
				: m_id(id_){}

			KeyEvent &keyEvent() noexcept{ return m_keyEvent; }

		private:
			std::uint32_t m_id;
			KeyEvent m_keyEvent;
			List<KeyEventFn> m_keyFns;
	};

	class Mouse: public Managed<Mouse, &Manager::doCreateMouse>{
		public:
			using ButtonEvent = Event<MouseButton, bool>;
			using MoveEvent = Event<Int32, Int32>;
			using ScrollEvent = Event<Int32, Int32>;

			explicit Mouse(std::uint32_t id_) noexcept
				: m_id(id_){}

			virtual void setCapture(bool enabled = true){}
			virtual void setRelativeMode(bool enabled = true){}

			ButtonEvent &buttonEvent() noexcept{ return m_btnEvent; }
			MoveEvent &moveEvent() noexcept{ return m_moveEvent; }
			ScrollEvent &scrollEvent() noexcept{ return m_scrollEvent; }

		private:
			std::uint32_t m_id;
			ButtonEvent m_btnEvent;
			MoveEvent m_moveEvent;
			ScrollEvent m_scrollEvent;
	};

	enum class GamepadButton{
		A, B, X, Y,

		cross = A, circle = B, square = X, triangle = Y,

		dpadLeft, dpadRight, dpadUp, dpadDown,

		count
	};

	class Gamepad: public Managed<Gamepad, &Manager::doCreateGamepad>{
		public:
			using ButtonEvent = Event<GamepadButton, bool>;

			explicit Gamepad(std::uint32_t id_) noexcept
				: m_id(id_){}

			std::uint32_t id() const noexcept{ return m_id; }

			ButtonEvent &buttonEvent() noexcept{ return m_buttonEvent; }

		private:
			std::uint32_t m_id;
			ButtonEvent m_buttonEvent;
	};
}

#define GPWE_INPUT_PLUGIN(type, name, author, major, minor, patch)\
	GPWE_PLUGIN(InputManager, type, name, author, major, minor, patch)

#endif // !GPWE_INPUT_HPP

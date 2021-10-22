#ifndef GPWE_EVENT_HPP
#define GPWE_EVENT_HPP 1

#include "Fn.hpp"
#include "List.hpp"
#include "algo.hpp"

namespace gpwe{
	template<typename ... Vals>
	class Event{
		public:
			using Slot = Fn<void(Vals...)>;
			using SlotIt = typename List<Slot>::const_iterator;

			template<typename F>
			SlotIt addFn(F &&f){
				return m_slots.emplace(m_slots.end(), std::forward<F>(f));
			}

			void removeFn(SlotIt f){
				m_slots.erase(f);
			}

			template<typename ... UVals>
			void emit_(UVals &&... vals){
				for(auto &&f : m_slots){
					f(std::forward<UVals>(vals)...);
				}
			}

		private:
			List<Slot> m_slots;
	};
}

#endif // !GPWE_EVENT_HPP

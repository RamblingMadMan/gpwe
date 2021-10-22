#ifndef GPWE_TIMER_HPP
#define GPWE_TIMER_HPP 1

#include "algo.hpp"
#include "Fn.hpp"
#include "Ticker.hpp"
#include "List.hpp"

namespace gpwe{
	class Timer{
		public:
			using TickFn = Fn<void(float)>;
			using FnIterator = typename List<TickFn>::const_iterator;

			explicit Timer(float dtMin_ = 0.f) noexcept
				: m_dtMin(dtMin_){}

			float dtMin() const noexcept{ return m_dtMin; }

			void setDtMin(float dtMin_) noexcept{
				m_dtMin = dtMin_;
			}

			void update(){
				m_dtAccum += m_ticks.tick();
				if(m_dtAccum >= m_dtMin){
					for(auto &&f : m_tickFns){
						f(m_dtAccum);
					}

					m_dtAccum = 0.f;
				}
			}

			void reset(){
				m_dtAccum = 0.f;
				m_ticks.reset();
			}

			template<typename F>
			FnIterator addTickFn(F &&f){
				return m_tickFns.insert(m_tickFns.end(), std::forward<F>(f));
			}

			void removeTickFn(FnIterator it){
				m_tickFns.erase(it);
			}

		private:
			float m_dtMin, m_dtAccum;
			Ticker<> m_ticks;
			List<TickFn> m_tickFns;
	};
}

#endif // !GPWE_TIMER_HPP

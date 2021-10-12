#ifndef GPWE_TICKER_HPP
#define GPWE_TICKER_HPP 1

#include <chrono>

namespace gpwe{
	using Clock = std::chrono::high_resolution_clock;
	using Seconds = std::chrono::duration<float>;

	template<typename Clock_ = Clock, typename Duration = Seconds>
	class Ticker{
		public:
			Ticker() noexcept
				: m_start(Clock_::now())
				, m_loop(m_start){}

			void reset(){
				m_start = Clock_::now();
				m_loop = m_start;
			}

			typename Duration::rep elapsed() const noexcept{
				auto now = Clock_::now();
				Duration diff = now - m_start;
				return diff.count();
			}

			typename Duration::rep tick() noexcept{
				auto loopEnd = Clock_::now();
				Duration loopDiff = loopEnd - m_loop;
				m_loop = loopEnd;
				return loopDiff.count();
			}

		private:
			typename Clock_::time_point m_start, m_loop;
	};
}

#endif // !GPWE_TICKER_HPP

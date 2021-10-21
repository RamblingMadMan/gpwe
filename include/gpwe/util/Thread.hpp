#ifndef GPWE_THREAD_HPP
#define GPWE_THREAD_HPP 1

#include <atomic>

#include "gpwe/log.hpp"

#include "pthread.h"

#include "Fn.hpp"

namespace gpwe{
	class Thread{
		public:
			Thread() noexcept = default;

			explicit Thread(Fn<void()> f);

			Thread(Thread &&other) noexcept
				: m_id(other.m_id.exchange(0))
				, m_fn(std::move(other.m_fn))
			{}

			~Thread(){
				join();
			}

			Thread &operator=(Thread &&other) noexcept{
				if(&other != this){
					auto otherId = other.m_id.exchange(0);
					join();
					m_id = otherId;
					m_fn = std::move(other.m_fn);
				}

				return *this;
			}

			Str name() const noexcept;

			void setName(StrView name_);

			void join();

		private:
			static void *pthread_routine(void *arg){
				auto fn = reinterpret_cast<Fn<void()>*>(arg);
				(*fn)();
				return nullptr;
			}

			std::atomic<std::uint64_t> m_id = 0;
			Fn<void()> m_fn;
	};
}

#endif // !GPWE_THREAD_HPP

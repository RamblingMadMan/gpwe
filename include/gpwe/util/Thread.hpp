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

			template<typename F>
			explicit Thread(F &&f)
				: m_fn(std::forward<F>(f))
			{
				pthread_t id;
				auto err = pthread_create(&id, nullptr, pthread_routine, &m_fn);
				if(err){
					log::errorLn("Error in pthread_create: {}", strerror(err));
					return;
				}

				m_thd = id;
			}

			Thread(Thread &&other) noexcept
				: m_thd(other.m_thd.exchange(0))
				, m_fn(std::move(other.m_fn))
			{}

			~Thread(){
				join();
			}

			Thread &operator=(Thread &&other) noexcept{
				if(&other != this){
					auto otherId = other.m_thd.exchange(0);
					join();
					m_thd = otherId;
				}

				return *this;
			}

			void join(){
				if(auto id = m_thd.exchange(0)){
					void *ret;
					pthread_join(id, &ret);
				}
			}

		private:
			static void *pthread_routine(void *arg){
				auto fn = reinterpret_cast<Fn<void()>*>(arg);
				(*fn)();
				return nullptr;
			}

			std::atomic<pthread_t> m_thd = 0;
			Fn<void()> m_fn;
	};
}

#endif // !GPWE_THREAD_HPP

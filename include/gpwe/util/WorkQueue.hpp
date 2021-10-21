#ifndef GPWE_WORKQUEUE_HPP
#define GPWE_WORKQUEUE_HPP 1

#include <future>
#include <mutex>

#include "List.hpp"
#include "Fn.hpp"

namespace gpwe{
	class WorkQueue{
		public:
			~WorkQueue(){
				doWork();
			}

			template<typename F>
			auto emplace(F f){
				using Result = std::result_of_t<F>;

				std::promise<Result> prom;
				auto fut = prom.get_future();

				std::lock_guard lock(m_mut);

				m_tasks.emplace_back([fn{std::move(f)}, p{std::move(prom)}]{
					if constexpr(std::is_same_v<void, Result>){
						fn();
						p.set_value();
					}
					else{
						p.set_value(fn());
					}
				});

				return fut;
			}

			std::size_t doWork(std::size_t n = SIZE_MAX){
				std::lock_guard lock(m_mut);

				if(m_tasks.empty()) return 0;

				n = std::min(n, m_tasks.size());

				auto it = m_tasks.begin();

				for(std::size_t i = 0; i < n && it != m_tasks.end(); i++){
					auto &&f = *it;
					f();
					auto next = it;
					++next;
					m_tasks.erase(it);
					it = next;
				}

				return n;
			}

			template<typename F>
			std::size_t doWorkOr(F &&f, std::size_t n = SIZE_MAX){
				auto ret = doWork(n);
				if(!ret){
					f();
				}

				return ret;
			}

		private:
			std::mutex m_mut;
			List<Fn<void()>> m_tasks;
	};
}

#endif // !GPWE_WORKQUEUE_HPP

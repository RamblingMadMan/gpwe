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

			void doWork(){
				std::lock_guard lock(m_mut);

				if(m_tasks.empty()) return;

				for(auto &&fn : m_tasks){
					fn();
				}

				m_tasks.clear();
			}

			template<typename F>
			void doWorkOr(F &&f){
				std::lock_guard lock(m_mut);
				if(m_tasks.empty()){
					f();
				}
				else{
					for(auto &&fn : m_tasks){
						fn();
					}

					m_tasks.clear();
				}
			}

		private:
			std::mutex m_mut;
			List<Fn<void()>> m_tasks;
	};
}

#endif // !GPWE_WORKQUEUE_HPP

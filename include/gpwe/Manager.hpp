#ifndef GPWE_MANAGER_HPP
#define GPWE_MANAGER_HPP 1

#include "Allocator.hpp"

namespace gpwe{
	namespace detail{
		template<typename T, typename ArgsPack>
		class ManagerCreator{
			public:
				virtual UniquePtr<T> doCreate(Args ... args) = 0;
		};
	}

	template<typename Derived, typename ... Ts>
	class Manager: protected detail::ManagerCreator<Ts>...{
		public:
			virtual ~Manager() = default;

			virtual void update(float dt){}

			template<typename T, typename ... Args>
			T *create(Args &&... args){
			}
	};
}

#endif // !GPWE_MANAGER_HPP 1

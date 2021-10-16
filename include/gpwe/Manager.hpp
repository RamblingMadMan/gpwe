#ifndef GPWE_MANAGER_HPP
#define GPWE_MANAGER_HPP 1

#include "List.hpp"

namespace gpwe{
	namespace detail{
		template<typename T>
		class ManagerStorage{
			protected:
				List<UniquePtr<T>> &ptrs() noexcept{ return m_ptrs; }
				const List<UniquePtr<T>> &ptrs() const noexcept{ return m_ptrs; }

				List<UniquePtr<T>> m_ptrs;
		};
	}

	template<typename ParentT, auto CreateFn>
	class Managed{
		public:
			using Parent = ParentT;

			static constexpr auto createFn() noexcept{ return CreateFn; }

			virtual ~Managed() = default;
	};

	template<typename Derived, typename ... Ts>
	class Manager: public detail::ManagerStorage<Ts>...{
		public:
			virtual ~Manager() = default;

			virtual void init(){}

			virtual void update(float dt){}

			template<typename T, typename ... Args>
			T *create(Args &&... args){
				auto self = static_cast<Derived*>(this);
				constexpr auto createFn = T::createFn();
				return insertUnique((self->*createFn)(std::forward<Args>(args)...));
			}

			template<typename T>
			bool destroy(T *ptr){
				return eraseUnique(ptr);
			}

		protected:
			template<typename T>
			List<UniquePtr<T>> &managed() noexcept{
				using namespace detail;
				return this->ManagerStorage<T>::ptrs();
			}

			template<typename T>
			const List<UniquePtr<T>> &managed() const noexcept{
				using namespace detail;
				return this->ManagerStorage<T>::ptrs();
			}

		private:
			template<typename T>
			T *insertUnique(UniquePtr<T> ptr){
				if(!ptr) return nullptr;
				List<UniquePtr<T>> &ptrs = this->detail::ManagerStorage<T>::ptrs();
				auto it = std::upper_bound(ptrs.begin(), ptrs.end(), ptr);
				auto ret = ptr.get();
				ptrs.insert(it, std::move(ptr));
				return ret;
			}

			template<typename T>
			bool eraseUnique(T *ptr){
				if(!ptr) return true;

				auto &&ptrs = this->detail::ManagerStorage<T>::ptrs;

				auto it = binary_find(
					ptrs.begin(), ptrs.end(), ptr,
					[](auto &&lhs, auto &&rhs){
						using Lhs = std::remove_cv_t<std::remove_reference_t<decltype(lhs)>>;
						using Rhs = std::remove_cv_t<std::remove_reference_t<decltype(rhs)>>;

						if constexpr(std::is_same_v<Lhs, Rhs>){
							return lhs < rhs;
						}
						else if constexpr(std::is_same_v<Lhs, UniquePtr<T>>){
							return lhs.get() < rhs;
						}
						else if constexpr(std::is_same_v<Rhs, UniquePtr<T>>){
							return lhs < rhs.get();
						}
					}
				);

				if(it != ptrs.end()){
					ptrs.erase(it);
					return true;
				}

				return false;
			}

			template<class Iter, class T, class Compare>
			static inline Iter binary_find(Iter begin, Iter end, T val, Compare cmp){
				Iter i = std::lower_bound(begin, end, val, cmp);

				if(i != end && !cmp(val, *i)) return i;
				else return end;
			}
	};
}

#endif // !GPWE_MANAGER_HPP 1

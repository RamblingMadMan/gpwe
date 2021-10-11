#ifndef GPWE_ALLOCATOR_HPP
#define GPWE_ALLOCATOR_HPP 1

#include <memory>

#include "sys.hpp"

namespace gpwe{
	template<typename T>
	class Allocator{
		public:
			using allocator_type = Allocator<T>;
			using value_type = T;
			using pointer = T*;
			using const_pointer = const T*;
			using size_type = std::size_t;

			template<typename U>
			struct rebind{
				using other = Allocator<U>;
			};

			Allocator() noexcept{}

			Allocator(const Allocator &a) noexcept = default;

			template<typename U>
			Allocator(const Allocator<U> &a) noexcept{}

			~Allocator() = default;

			template<typename ... Args>
			void construct(pointer p, Args &&... args) noexcept(noexcept(T(std::forward<Args>(args)...))){
				new(p) T(std::forward<Args>(args)...);
			}

			void destroy(pointer p){
				p->~T();
			}

			pointer allocate(size_type n){
				return reinterpret_cast<pointer>(sys::alloc(n * sizeof(T)));
			}

			void deallocate(pointer p, size_type n){
				sys::free(p);
			}

			void deallocate(pointer p){
				sys::free(p);
			}
	};

	template<typename T>
	class UniquePtr{
		public:
			UniquePtr() noexcept
				: m_ptr(nullptr){}

			UniquePtr(std::nullptr_t) noexcept
				: UniquePtr(){}

			template<typename U>
			explicit UniquePtr(U *ptr) noexcept
				: m_ptr(ptr){}

			template<typename U>
			UniquePtr(UniquePtr<U> &&other) noexcept
				: m_ptr(other.release()){}

			~UniquePtr(){
				destroy();
			}

			template<typename U>
			UniquePtr &operator=(UniquePtr<U> &&other) noexcept{
				if((void*)this != (void*)&other){
					destroy();
					m_ptr = other.release();
				}

				return *this;
			}

			operator bool() const noexcept{ return !!m_ptr; }

			T *operator->() noexcept{ return m_ptr; }
			const T *operator->() const noexcept{ return m_ptr; }

			T *get() noexcept{ return m_ptr; }
			const T *get() const noexcept{ return m_ptr; }

			T *release() noexcept{
				auto ret = m_ptr;
				m_ptr = nullptr;
				return ret;
			}

			void reset(T *ptr = nullptr) noexcept{
				destroy();
				m_ptr = ptr;
			}

		private:
			void destroy(){
				if(!m_ptr) return;
				Allocator<T> alloc;
				alloc.destroy(m_ptr);
				alloc.deallocate(m_ptr);
			}

			T *m_ptr;
	};

	template<typename T>
	UniquePtr(T*) -> UniquePtr<T>;

	template<typename T, typename ... Args>
	UniquePtr<T> makeUnique(Args &&... args){
		Allocator<T> alloc;
		auto p = alloc.allocate(1);
		alloc.construct(p, std::forward<Args>(args)...);
		return UniquePtr(p);
	}
}

#endif // !GPWE_ALLOCATOR_HPP

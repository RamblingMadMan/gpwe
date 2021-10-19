#ifndef GPWE_ALLOCATOR_HPP
#define GPWE_ALLOCATOR_HPP 1

#include <atomic>
#include <memory>
#include <variant>

#include "gpwe/config.hpp"

#include "meta.hpp"

namespace gpwe{
	namespace sys{
		void *alloc(std::size_t n);
		void free(void *ptr);
	}

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

			Allocator() noexcept = default;

			Allocator(const Allocator &a) noexcept = default;

			template<typename U>
			Allocator(const Allocator<U> &a) noexcept{}

			~Allocator() = default;

			template<typename ... Args>
			void construct(pointer p, Args &&... args) noexcept(noexcept(T{std::forward<Args>(args)...})){
				new(p) T(std::forward<Args>(args)...);
			}

			void destroy(pointer p) noexcept{
				p->~T();
			}

			pointer allocate(size_type n) noexcept{
				return reinterpret_cast<pointer>(sys::alloc(n * sizeof(T)));
			}

			void deallocate(pointer p, size_type n) noexcept{
				sys::free(p);
			}

			void deallocate(pointer p) noexcept{
				sys::free(p);
			}
	};

	using SmallBuffer = char[GPWE_STATIC_BUFFER_SIZE];

	using InPlaceT = std::in_place_t;
	inline InPlaceT InPlace;

	template<typename T>
	class UniquePtr{
		public:
			UniquePtr() noexcept
				: m_ptr(nullptr){}

			template<typename ... Args>
			UniquePtr(InPlaceT, Args &&... args)
				noexcept(noexcept(T(std::forward<Args>(args)...)))
			{
				construct(std::forward<Args>(args)...);
			}

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
					static Allocator<T> alloc;
					auto otherPtr = other.m_ptr.exchange(nullptr);
					reset(otherPtr);
				}

				return *this;
			}

			template<typename U>
			UniquePtr &operator=(U *ptr) noexcept{
				if((void*)ptr != (void*)m_ptr){
					reset(ptr);
				}

				return *this;
			}

			template<typename U>
			inline bool operator<(const UniquePtr<U> &other) const noexcept{
				return m_ptr < other.m_ptr;
			}

			template<typename U>
			inline bool operator<(const U *other) const noexcept{
				return m_ptr < other;
			}

			inline operator bool() const noexcept{ return !!m_ptr; }

			inline T *operator->() noexcept{ return m_ptr; }
			inline const T *operator->() const noexcept{ return m_ptr; }

			T *get() noexcept{ return m_ptr; }
			const T *get() const noexcept{ return m_ptr; }

			T *release() noexcept{
				return m_ptr.exchange(nullptr);
			}

			void reset(T *ptr = nullptr) noexcept{
				static Allocator<T> alloc;
				if(auto old = m_ptr.exchange(ptr)){
					alloc.destroy(old);
					alloc.deallocate(old);
				}
			}

		private:
			template<typename ... Args>
			void construct(Args &&... args){
				static Allocator<T> alloc;
				auto ptr = alloc.allocate(1);
				alloc.construct(ptr, std::forward<Args>(args)...);
				if(auto oldPtr = m_ptr.exchange(ptr)){
					alloc.destroy(oldPtr);
					alloc.deallocate(oldPtr);
				}
			}

			void destroy(){
				static Allocator<T> alloc;
				if(auto ptr = m_ptr.exchange(nullptr)){
					alloc.deallocate(ptr);
					alloc.destroy(ptr);
				}
			}

			std::atomic<T*> m_ptr = nullptr;

			template<typename U>
			friend class UniquePtr;
	};

	template<typename T>
	UniquePtr(T*) -> UniquePtr<T>;

	template<typename T, typename ... Args>
	UniquePtr<T> makeUnique(Args &&... args){
		return UniquePtr<T>(InPlace, std::forward<Args>(args)...);
	}
}

#endif // !GPWE_ALLOCATOR_HPP

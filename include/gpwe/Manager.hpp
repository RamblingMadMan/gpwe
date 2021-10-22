#ifndef GPWE_MANAGER_HPP
#define GPWE_MANAGER_HPP 1

#include "util/Object.hpp"
#include "util/algo.hpp"
#include "util/List.hpp"
#include "util/Str.hpp"
#include "Version.hpp"

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

	// f$%*n diamond inheritance
	class ManagedBase: public virtual ObjectBase{
		public:
			virtual ~ManagedBase() = default;
	};

	template<auto ObjName, auto CreateFn, typename ... Props>
	class Managed: public Object<ObjName, Props...>, public ManagedBase{
		public:
			static constexpr auto createFn() noexcept{ return CreateFn; }

			virtual ~Managed() = default;
	};

	enum class ManagerKind: std::uint16_t{
		data = 1,
		plugin = 1 << 1,
		app = 1 << 2,
		render = 1 << 3,
		physics = 1 << 4,
		input = 1 << 5,
		sys = 1 << 6,
		log = 1 << 7,
		world = 1 << 8
	};

	class ManagerBase{
		public:
			virtual ~ManagerBase() = default;
	};

	template<typename Derived, ManagerKind Kind_, typename ... Ts>
	class Manager: public ManagerBase, public detail::ManagerStorage<Ts>...{
		public:
			virtual ~Manager() = default;

			static ManagerKind managerKind() noexcept{ return Kind_; }

			virtual void init(){}

			virtual void update(float dt){}

			template<typename T, typename ... Args>
			T *create(Args &&... args){
				auto self = static_cast<Derived*>(this);
				constexpr auto createFn = T::createFn();

				if constexpr(std::is_member_function_pointer_v<decltype(createFn)>){
					return insertUnique((self->*createFn)(std::forward<Args>(args)...));
				}
				else{
					return insertUnique(createFn(std::forward<Args>(args)...));
				}
			}

			template<typename T>
			bool destroy(T *ptr){
				return eraseUnique(ptr);
			}

			template<typename T>
			std::size_t numManaged() const noexcept{
				using namespace detail;
				return this->ManagerStorage<T>::ptrs().size();
			}

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

				auto &&ptrs = this->detail::ManagerStorage<T>::ptrs();

				auto it = binaryFind(ptrs, ptr);

				if(it != ptrs.end()){
					ptrs.erase(it);
					return true;
				}

				return false;
			}
	};
}

#define GPWE_PLUGIN(kind, type, name, author, major, minor, patch)\
extern "C" const char *gpwePlugin##Name(){ return name; }\
extern "C" const char *gpwePlugin##Author(){ return author; }\
extern "C" gpwe::Version gpwePlugin##Version(){ return { major, minor, patch }; }\
extern "C" gpwe::ManagerKind gpwePlugin##Kind(){ return gpwe::ManagerKind::kind; }\
extern "C" gpwe::ManagerBase *gpwePluginCreateManager(){\
	auto mem = gpwe::sys::alloc(sizeof(type));\
	return new(mem) type();\
}

#endif // !GPWE_MANAGER_HPP 1

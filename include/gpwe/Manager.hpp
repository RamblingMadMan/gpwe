#ifndef GPWE_MANAGER_HPP
#define GPWE_MANAGER_HPP 1

/**
 * @defgroup Manager Managers
 * @{
 */

#include "util/Object.hpp"
#include "util/algo.hpp"
#include "util/List.hpp"
#include "util/Str.hpp"
#include "Version.hpp"

namespace gpwe{
	// f$%*n diamond inheritance
	class ManagedBase: public virtual ObjectBase{
		public:
			virtual ~ManagedBase() = default;
	};

	/**
	 * @brief An object managed by a manager.
	 * @tparam Derived the type deriving this class.
	 * @tparam CreateFn the function used to create instances of \ref Derived
	 * @tparam Props a list of \ref Property types
	 */
	template<typename Derived, auto CreateFn, typename ... Props>
	class Managed: public Object<Derived, Props...>, public ManagedBase{
		public:
			using ObjectType = Object<Derived, Props...>;
			using ManagedType = Managed<Derived, CreateFn, Props...>;

			explicit Managed(ObjectBase *parent_ = nullptr)
				: ObjectType(parent_){}

			static constexpr auto createFn() noexcept{ return CreateFn; }

			virtual ~Managed() = default;
	};

	/**
	 * @brief Enum identifying different kinds of managers.
	 */
	enum class ManagerKind: std::uint16_t{
		data,
		plugin,
		app,
		render,
		physics,
		input,
		sys,
		log,
		world,
		ui,

		count
	};

	/**
	 * @brief Base class of the real meat and two veg manager.
	 */
	class ManagerBase{
		public:
			virtual ~ManagerBase() = default;
	};

	namespace detail{
		template<typename T>
		class ManagerStorage{
			protected:
				List<UniquePtr<T>> &ptrs() noexcept{ return m_ptrs; }
				const List<UniquePtr<T>> &ptrs() const noexcept{ return m_ptrs; }

				List<UniquePtr<T>> m_ptrs;
		};
	}

	/**
	 * @brief Manager class used throughout the engine to manage objects.
	 * @tparam Derived The type deriving this class. curious...
	 * @tparam Kind_ Self-explanatory
	 * @tparam Ts List of object types managed by the manager
	 */
	template<typename Derived, ManagerKind Kind_, typename ... Ts>
	class Manager: public ManagerBase, public detail::ManagerStorage<Ts>...{
		public:
			virtual ~Manager() = default;

			static ManagerKind managerKind() noexcept{ return Kind_; }

			virtual void init(){}

			virtual void update(float dt){}

			template<typename T, typename ... Args>
			T *create(Args &&... args){
				static_assert(
					hasManaged<T>(),
					"Type is not a base of one of the managed types"
				);

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

			template<typename T>
			static constexpr bool hasManaged(){
				constexpr auto tRep = meta::repeat(meta::type<T>, meta::value<sizeof...(Ts)>);

				return meta::anyOf(
					meta::zip(
						meta::fn<meta::IsBaseOf>,
						meta::types<Ts...>,
						meta::repeat(meta::type<T>, meta::value<sizeof...(Ts)>)
					)
				);
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

/**
 * @brief Defines a plugin interface within a plugin.
 * @param kind gpwe::ManagerKind without the prefix
 * @param type the type of the new manager
 * @param name the name of the plugin
 * @param author the original creator of the plugin
 * @param major major version
 * @param minor minor version
 * @param patch patch version
 * @note Versions are expected to follow <a href="https://semver.org">semantic versioning</a>.
 */
#define GPWE_PLUGIN(kind, type, name, author, major, minor, patch)\
extern "C" const char *gpwePlugin##Name(){ return name; }\
extern "C" const char *gpwePlugin##Author(){ return author; }\
extern "C" gpwe::Version gpwePlugin##Version(){ return { major, minor, patch }; }\
extern "C" gpwe::ManagerKind gpwePlugin##Kind(){ return gpwe::ManagerKind::kind; }\
extern "C" gpwe::ManagerBase *gpwePluginCreateManager(){\
	auto mem = gpwe::sys::alloc(sizeof(type));\
	return new(mem) type();\
}

/**
 * @}
 */

#endif // !GPWE_MANAGER_HPP 1

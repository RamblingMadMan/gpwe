#ifndef GPWE_META_HPP
#define GPWE_META_HPP 1

#include <typeindex>
#include <type_traits>

/**
 * @defgroup Meta Compile-time utilities
 * @{
 */

#include "types.hpp"
#include "math.hpp"

namespace gpwe{
	class ObjectBase;
}

namespace gpwe::meta{

	/**
	 * @defgroup MetaTypes Meta-types
	 * @{
	 */

	template<typename ... Xs>
	struct Types{};

	template<typename T>
	using Type = Types<T>;

	template<template<typename...> typename ... Xs>
	struct Fns{};

	template<template<typename...> typename F>
	using Fn = Fns<F>;

	template<Nat64 N>
	struct CStr;

	template<typename T>
	struct TypeName;

	template<typename T>
	struct TypeInfo;

	template<auto Val>
	struct Value;

	template<auto ... Vals>
	using Vector = Types<Value<Vals>...>;

	/**
	 * @}
	 */

	/**
	 * @defgroup MetaFns Meta-functions
	 * @{
	 */

	template<typename Xs>
	struct HeadT;

	template<typename Xs>
	using Head = typename HeadT<Xs>::type;

	template<typename Xs>
	struct TailT;

	template<typename Xs>
	using Tail = typename TailT<Xs>::type;

	template<Nat64 Idx, typename Xs>
	struct AtT;

	template<Nat64 Idx, typename Xs>
	using At = typename AtT<Idx, Xs>::type;

	template<typename X, typename Ys>
	struct AppendT;

	template<typename X, typename Ys>
	using Append = typename AppendT<X, Ys>::type;

	template<typename X, typename Ys>
	struct PrependT;

	template<typename X, typename Ys>
	using Prepend = typename PrependT<X, Ys>::type;

	template<typename X, typename Ys>
	struct IsIn;

	template<template<class...> typename F, typename Types>
	struct ApplyT;

	template<template<class...> typename F, typename Types>
	using Apply = typename ApplyT<F, Types>::type;

	template<template<class...> typename F, typename Types>
	struct MapT;

	template<template<class...> typename F, typename Types>
	using Map = typename MapT<F, Types>::type;

	template<typename T>
	using IsObject = Value<std::is_base_of_v<ObjectBase, T>>;

	template<typename Obj>
	struct ObjectManagerT;

	template<typename Obj>
	using ObjectManager = typename ObjectManagerT<Obj>::type;

	template<template<class...> typename T, typename Types>
	struct InstantiateT;

	template<template<class...> typename T, typename Types>
	using Instantiate = typename InstantiateT<T, Types>::type;

	/**
	 * @}
	 */

	/**
	 * @defgroup MetaWrappers Meta-function wrappers
	 * @{
	 */

	/**
	 * @brief Utility for creating a type list.
	 */
	template<typename ... Ts>
	inline constexpr Types<Ts...> types;

	/**
	 * @brief Utility for creating a type handle.
	 */
	template<typename T>
	inline constexpr Type<T> type;

	template<template<typename...> typename ... Fs>
	inline constexpr Fns<Fs...> fns;

	template<template<typename...> typename F>
	inline constexpr Fn<F> fn;

	/**
	 *@brief Utility to create a list of values.
	 */
	template<auto ... Xs>
	inline constexpr Vector<Xs...> vector;

	template<typename X, typename ... Xs>
	inline constexpr auto append(Type<X>, Types<Xs...>){
		return Append<X, Types<Xs...>>{};
	}

	template<typename X, typename ... Xs>
	inline constexpr auto prepend(Type<X>, Types<Xs...>){
		return Prepend<X, Types<Xs...>>{};
	}

	template<typename ... Xs>
	inline constexpr auto head(Types<Xs...>){
		return Head<Types<Xs...>>{};
	}

	template<typename ... Xs>
	inline constexpr auto tail(Types<Xs...>){
		return Tail<Types<Xs...>>{};
	}

	template<typename X, typename ... Xs>
	inline constexpr bool isIn(Type<X>, Types<Xs...>){
		return IsIn<X, Types<Xs...>>{};
	}

	template<template<typename...> typename F, typename ... Xs>
	inline constexpr auto apply(Fn<F>, Types<Xs...>){
		return Apply<F, Types<Xs...>>{};
	}

	template<template<typename...> typename F, typename ... Xs>
	inline constexpr auto map(Fn<F>, Types<Xs...>){
		return Map<F, Types<Xs...>>{};
	}

	template<typename T>
	inline constexpr bool isObject(Type<T>){
		return IsObject<T>{};
	}

	template<typename Obj>
	inline constexpr auto objectManager(Type<Obj>){
		return ObjectManager<Obj>{};
	}

	/**
	 * @}
	 */

	/**
	 * @defgroup MetaImpl Implementation details
	 * @warning This code is write-only
	 * @{
	 */

	template<typename X, typename ... Xs>
	struct HeadT<Types<X, Xs...>>{
		using Result = X;
		using type = X;
	};

	template<typename X, typename ... Xs>
	struct TailT<Types<X, Xs...>>{
		using Result = Types<Xs...>;
		using type = Result;
	};

	template<Nat64 Idx, typename X, typename ... Xs>
	struct AtT<Idx, Types<X, Xs...>>{
		using Result = AtT<Idx-1, Types<Xs...>>;
		using type = Result;
	};

	template<typename X, typename ... Xs>
	struct AtT<0, Types<X, Xs...>>{
		using Result = X;
		using type = Result;
	};

	template<typename X, typename ... Ys>
	struct AppendT<X, Types<Ys...>>{
		using Result = Types<Ys..., X>;
		using type = Result;
	};

	template<typename X, typename ... Ys>
	struct PrependT<X, Types<Ys...>>{
		using Result = Types<X, Ys...>;
		using type = Result;
	};

	template<auto Val>
	struct Value{
		using Type = decltype(Val);
		using type = Type;
		constexpr operator Type() const noexcept{ return value(); }
		static constexpr Type value(){ return Val; };
	};

	template<typename X>
	struct IsIn<X, Types<>>: Value<false>{};

	template<typename X, typename Y, typename ... Ys>
	struct IsIn<X, Types<Y, Ys...>>: Value<
		std::is_same_v<X, Y> || IsIn<X, Types<Ys...>>::value()
	>{};

	namespace detail{
		constexpr Nat64 cStrLen(char const *str, Nat64 count = 0){
			return ('\0' == str[0]) ? count : cStrLen(str+1, count+1);
		}

		constexpr Int64 cStrCmp(char const *a, char const *b, Nat64 len, Nat64 count = 0){
			if(count == len) return 0;

			auto d = *a - *b;
			if(d == 0){
				return cStrCmp(a+1, b+1, len, count+1);
			}
			else if(*a == '\0'){
				return -(len - count);
			}
			else if(*b == '\0'){
				return len - count;
			}
			else{
				return d;
			}
		}

		constexpr Nat64 cStrCpy(char *src, char const *dest, Nat64 len, Nat64 count = 0){
			if(count == len) return count;
			*src = *dest;
			return cStrCpy(src+1, dest+1, len, count+1);
		}

		template<typename T, typename U>
		constexpr std::common_type_t<T, U> min(T a, U b){
			return a < b ? a : b;
		}

		template<typename T, typename U>
		constexpr std::common_type_t<T, U> max(T a, U b){
			return a < b ? b : a;
		}
	}

	template<Nat64 N>
	struct CStr{
		char str[N + 1]{};
		static constexpr Nat64 length = N;

		constexpr CStr(char const* s){
			for(Nat64 i = 0; i != N; ++i) str[i] = s[i];
		}

		constexpr CStr(const CStr<N> &other){
			for(Nat64 i = 0; i != N; ++i) str[i] = other.str[i];
		}

		template<Nat64 UN>
		constexpr bool operator==(const CStr<UN> &other) const noexcept{
			const auto n = detail::max(N, UN); // yes, max
			return detail::cStrCmp(str, other.str, n) == 0;
		}

		template<Nat64 UN>
		constexpr bool operator<(const CStr<UN> &other) const noexcept{
			const auto n = detail::max(N, UN);
			return detail::cStrCmp(str, other.str, n) < 0;
		}

		constexpr operator char const*() const{ return str; }
	};

	template<Nat64 N>
	CStr(char const (&)[N]) -> CStr<N - 1>;

	namespace literals{
		template<CStr Str>
		constexpr auto operator""_cs(){
			return Str;
		}
	}

	namespace detail{
		template<typename T, typename = void>
		struct TypeNameHelper;
	}

	template<typename T>
	struct TypeName{
		static constexpr auto Name = detail::TypeNameHelper<T>::Name;
	};

	template<typename T>
	struct TypeInfo{
		static constexpr auto id(){ return typeid(T); }
		static constexpr auto index(){ return std::type_index(id()); }
		static constexpr auto name(){ return TypeName<T>::Name; }
		static constexpr std::size_t size(){ return sizeof(T); }
	};

	namespace detail{
		template<typename ObjBase>
		struct ObjectBaseManagerHelper;

		template<typename Obj, typename = void>
		struct ObjectManagerHelper;
	}

	template<typename Obj>
	struct ObjectManagerT{
		static_assert(isObject(type<Obj>));

		using Result = typename detail::ObjectManagerHelper<Obj>::type;
		using type = Result;

		static type *get() noexcept{ return detail::ObjectManagerHelper<Obj>::get(); }
	};

	template<std::size_t Idx>
	struct Tag{};

	namespace tags{
		using _0 = Tag<0>;
		using _1 = Tag<1>;
		using _2 = Tag<2>;
		using _3 = Tag<3>;
		using _4 = Tag<4>;
		using _5 = Tag<5>;
		using _6 = Tag<6>;
		using _7 = Tag<7>;
		using _8 = Tag<8>;
		using _9 = Tag<9>;
	}

	namespace detail{
		template<typename T>
		struct IsCompleteHelper{
			template <typename U>
			static auto test(U*)  -> std::integral_constant<bool, sizeof(U) == sizeof(U)>;
			static auto test(...) -> std::false_type;

			using Value = meta::Value<std::is_same_v<decltype(test((T*)0)), std::true_type>>;
		};
	}

	template<typename T>
	struct IsCompleteT: detail::IsCompleteHelper<T>::Value{};

	template<typename T>
	constexpr auto IsComplete = Value<IsCompleteT<T>::value>{};

	template<template<class...> typename F, typename ... Xs>
	struct ApplyT<F, Types<Xs...>>{
		using Result = typename F<Xs...>::type;
		using type = Result;
	};

	template<template<class...> typename F, typename ... Xs>
	struct MapT<F, Types<Xs...>>{
		using Result = Types<typename F<Xs>::Result...>;
		using type = Result;
	};

	template<template<class...> typename T, typename ... Xs>
	struct InstantiateT<T, Types<Xs...>>{
		using Result = T<Xs...>;
		using type = Result;
	};
}

namespace gpwe{
	namespace literals{
		using namespace meta::literals;
	}

	using namespace literals;
}

#define GPWE_REGISTER_TYPE(type)\
	template<> struct gpwe::meta::detail::TypeNameHelper<type>{ static constexpr auto Name = CStr(#type); }

#define GPWE_REGISTER_MANAGED(obj, manager)\
	GPWE_REGISTER_TYPE(obj);\
	template<> struct gpwe::meta::detail::ObjectBaseManagerHelper<obj>{ static manager *get(); };\
	template<typename Obj> struct gpwe::meta::detail::ObjectManagerHelper<Obj, std::enable_if_t<std::is_base_of_v<obj, Obj>>>: ObjectBaseManagerHelper<obj>{ using type = manager; }

namespace gpwe{
	GPWE_REGISTER_TYPE(Nat8);
	GPWE_REGISTER_TYPE(Nat16);
	GPWE_REGISTER_TYPE(Nat32);

	GPWE_REGISTER_TYPE(Int8);
	GPWE_REGISTER_TYPE(Int16);
	GPWE_REGISTER_TYPE(Int32);

	GPWE_REGISTER_TYPE(Float32);
	GPWE_REGISTER_TYPE(Float64);

	GPWE_REGISTER_TYPE(Vec2);
	GPWE_REGISTER_TYPE(Vec3);
	GPWE_REGISTER_TYPE(Vec4);

	GPWE_REGISTER_TYPE(Mat2);
	GPWE_REGISTER_TYPE(Mat3);
	GPWE_REGISTER_TYPE(Mat4);

	GPWE_REGISTER_TYPE(ObjectBase);
	GPWE_REGISTER_TYPE(PropertyBase);

	GPWE_REGISTER_TYPE(Camera);

	GPWE_REGISTER_TYPE(SysManager);
	GPWE_REGISTER_TYPE(LogManager);
	GPWE_REGISTER_TYPE(ResourceManager);
	GPWE_REGISTER_TYPE(AppManager);

	GPWE_REGISTER_TYPE(InputManager);
	GPWE_REGISTER_TYPE(InputSystem);
	GPWE_REGISTER_MANAGED(InputKeyboard, InputManager);
	GPWE_REGISTER_MANAGED(InputMouse, InputManager);
	GPWE_REGISTER_MANAGED(InputGamepad, InputManager);

	GPWE_REGISTER_TYPE(RenderManager);
	GPWE_REGISTER_MANAGED(RenderFramebuffer, RenderManager);
	GPWE_REGISTER_MANAGED(RenderTexture, RenderManager);
	GPWE_REGISTER_MANAGED(RenderPipeline, RenderManager);
	GPWE_REGISTER_MANAGED(RenderProgram, RenderManager);
	GPWE_REGISTER_MANAGED(RenderGroup, RenderManager);
	GPWE_REGISTER_MANAGED(RenderInstance, RenderManager);
	GPWE_REGISTER_MANAGED(RenderInstanceData, RenderManager);

	GPWE_REGISTER_TYPE(PhysicsManager);
	GPWE_REGISTER_MANAGED(PhysicsWorld, PhysicsManager);
	GPWE_REGISTER_MANAGED(PhysicsBody, PhysicsManager);
	GPWE_REGISTER_MANAGED(PhysicsBodyShape, PhysicsManager);

	GPWE_REGISTER_TYPE(WorldManager);
	GPWE_REGISTER_MANAGED(WorldBlock, WorldManager);
	GPWE_REGISTER_MANAGED(WorldEntity, WorldManager);

	GPWE_REGISTER_TYPE(UiManager);
	GPWE_REGISTER_TYPE(UiWidgetBase);
	GPWE_REGISTER_MANAGED(UiSolidColor, UiManager);
	GPWE_REGISTER_MANAGED(UiLayout, UiManager);
}

/**
 * @}
 */

/**
 * @}
 */

#endif // !GPWE_META_HPP

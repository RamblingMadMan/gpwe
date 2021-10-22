#ifndef GPWE_META_HPP
#define GPWE_META_HPP 1

#include <typeindex>
#include <type_traits>

#include "types.hpp"
#include "math.hpp"

namespace gpwe::meta{
	template<typename...>
	struct Types{};

	template<typename T>
	struct Types<T>{
		template<typename ... Args>
		static T instantiate(Args &&... args){ return T(std::forward<Args>(args)...); }
	};

	template<typename T>
	using Type = Types<T>;

	template<typename ... Ts>
	constexpr Types<Ts...> types;

	template<typename T>
	constexpr Type<T> type;

	template<auto Val>
	struct Value{
		using Type = decltype(Val);
		using type = Type;
		constexpr operator Type() const noexcept{ return value(); }
		static constexpr Type value(){ return Val; };
	};

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
		template<typename T>
		struct TypeNameHelper;
	}

#define GDEF_TYPENAME(type)\
	template<> struct detail::TypeNameHelper<type>{ static constexpr auto Name = CStr(#type); }

	GDEF_TYPENAME(Nat8);
	GDEF_TYPENAME(Nat16);
	GDEF_TYPENAME(Nat32);

	GDEF_TYPENAME(Int8);
	GDEF_TYPENAME(Int16);
	GDEF_TYPENAME(Int32);

	GDEF_TYPENAME(Float32);
	GDEF_TYPENAME(Float64);

	GDEF_TYPENAME(Vec2);
	GDEF_TYPENAME(Vec3);
	GDEF_TYPENAME(Vec4);

	GDEF_TYPENAME(Mat2);
	GDEF_TYPENAME(Mat3);
	GDEF_TYPENAME(Mat4);

#undef GDEF_TYPENAME

	template<typename T>
	struct TypeName{
		static constexpr auto Name = detail::TypeNameHelper<T>::Name;
	};

	template<typename T>
	struct TypeInfo{
		static auto id(){ return typeid(T); }
		static auto index(){ return std::type_index(id()); }
		static auto name(){ return TypeName<T>::Name; }
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

	template<typename Types, template<class...> typename F>
	struct ApplyT;

	template<typename ... Xs, template<class...> typename F>
	struct ApplyT<Types<Xs...>, F>{
		using Result = typename F<Xs...>::type;
		using type = Result;
	};

	template<typename Types, template<class...> typename F>
	using Apply = typename ApplyT<Types, F>::type;

	template<typename Types, template<class...> typename F>
	struct MapT;

	template<typename ... Xs, template<class...> typename F>
	struct MapT<Types<Xs...>, F>{
		using Result = Types<typename F<Xs>::Result...>;
		using type = Result;
	};

	template<typename Types, template<class...> typename F>
	using Map = typename MapT<Types, F>::type;

	template<typename Types, template<class...> typename T>
	struct InstantiateT;

	template<typename ... Xs, template<class...> typename T>
	struct InstantiateT<Types<Xs...>, T>{
		using Result = T<Xs...>;
		using type = Result;
	};

	template<typename Types, template<class...> typename T>
	using Instantiate = typename InstantiateT<Types, T>::type;
}

namespace gpwe{
	namespace literals{
		using namespace meta::literals;
	}

	using namespace literals;
}

#endif // !GPWE_META_HPP

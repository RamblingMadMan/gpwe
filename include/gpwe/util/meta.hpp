#ifndef GPWE_META_HPP
#define GPWE_META_HPP 1

#include <type_traits>

namespace gpwe::meta{
	template<typename...>
	struct Types{};

	template<auto Val>
	struct Value{
		using Type = decltype(Val);
		constexpr operator Type() const noexcept{ return value(); }
		static constexpr Type value(){ return Val; };
	};

	template<std::size_t Idx>
	struct TagT{};

	namespace tags{
		using _0 = TagT<0>;
		using _1 = TagT<1>;
		using _2 = TagT<2>;
		using _3 = TagT<3>;
		using _4 = TagT<4>;
		using _5 = TagT<5>;
		using _6 = TagT<6>;
		using _7 = TagT<7>;
		using _8 = TagT<8>;
		using _9 = TagT<9>;
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
	struct IsComplete: detail::IsCompleteHelper<T>::Value{};
}

#endif // !GPWE_META_HPP

#ifndef GPWE_META_HPP
#define GPWE_META_HPP 1

namespace gpwe::meta{
	template<typename...>
	struct Types{};

	template<auto Val>
	struct Value{
		using Type = decltype(Val);
		static constexpr Type value(){ return Val; };
	};
}

#endif // !GPWE_META_HPP

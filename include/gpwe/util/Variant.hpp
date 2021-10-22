#ifndef GPWE_VARIANT_HPP
#define GPWE_VARIANT_HPP 1

#include <variant>

namespace gpwe{
	template<typename ... Ts>
	using Variant = std::variant<Ts...>;
}

#endif // !GPWE_VARIANT_HPP

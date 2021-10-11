#ifndef GPWE_VECTOR_HPP
#define GPWE_VECTOR_HPP 1

#include <vector>

#include "Allocator.hpp"

namespace gpwe{
	template<typename T>
	using Vector = std::vector<T, Allocator<T>>;
}

#endif // !GPWE_VECTOR_HPP

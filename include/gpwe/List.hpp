#ifndef GPWE_LIST_HPP
#define GPWE_LIST_HPP 1

#include "plf_list.h"

#include "Allocator.hpp"

namespace gpwe{
	template<typename T>
	using List = plf::list<T, Allocator<T>>;
}

#endif // !GPWE_LIST_HPP

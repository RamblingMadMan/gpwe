#ifndef GPWE_MAP_HPP
#define GPWE_MAP_HPP 1

#include <map>

#include "robin_hood.h"

#include "Allocator.hpp"

namespace gpwe{
	template<typename Key, typename Val, typename Compare = std::less<void>>
	using Map = std::map<Key, Val, Compare, Allocator<std::pair<const Key, Val>>>;

	template<typename Key, typename Val>
	using HashMap = robin_hood::unordered_map<Key, Val>;
}

#endif // !GPWE_MAP_HPP

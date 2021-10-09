#ifndef GPWE_SYS_HPP
#define GPWE_SYS_HPP 1

#include <cstdint>

namespace gpwe::sys{
	void init(std::uint16_t w, std::uint16_t h);
	int exec();
}

#endif // !GPWE_SYS_HPP

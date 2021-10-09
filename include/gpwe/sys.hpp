#ifndef GPWE_SYS_HPP
#define GPWE_SYS_HPP 1

#include <cstdint>

namespace gpwe{
	class Renderer;
}

namespace gpwe::sys{
	void init(std::uint16_t w, std::uint16_t h);
	int exec();

	Renderer *renderer();
}

#endif // !GPWE_SYS_HPP

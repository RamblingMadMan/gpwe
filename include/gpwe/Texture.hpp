#ifndef GPWE_TEXTURE_HPP
#define GPWE_TEXTURE_HPP 1

#include <cstdint>

namespace gpwe{
	class Texture{
		public:
			enum class Kind{
				// Color formats
				r8, rg8, rgb8, rgba8,
				r16, rg16, rgb16, rgba16,
				r16n, rg16n, rgb16n, rgba16n,
				r16i, rg16i, rgb16i, rgba16i,
				r16f, rg16f, rgb16f, rgba16f,
				r32n, rg32n, rgb32n, rgba32n,
				r32i, rg32i, rgb32i, rgba32i,
				r32f, rg32f, rgb32f, rgba32f,

				// Depth(+Stencil) formats
				d16,
				d32, d32f,
				d24s8,

				count
			};

			virtual ~Texture() = default;

			virtual Kind kind() const noexcept = 0;
	};

	class Texture2D: public Texture{
		public:
			virtual std::uint16_t width() const noexcept = 0;
			virtual std::uint16_t height() const noexcept = 0;
	};
}

#endif // !GPWE_TEXTURE_HPP

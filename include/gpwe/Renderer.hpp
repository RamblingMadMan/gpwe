#ifndef GPWE_RENDERER_HPP
#define GPWE_RENDERER_HPP 1

#include <cstdint>

#include "Shape.hpp"

namespace gpwe{
	class RenderGroup{
		public:
			virtual ~RenderGroup() = default;

			virtual void draw() const noexcept = 0;
			virtual void setNumInstances(std::uint32_t n) = 0;

		protected:
			virtual void *dataPtr(std::uint32_t idx) = 0;
	};

	class Renderer{
		public:
			virtual ~Renderer() = default;

			virtual void present() noexcept = 0;

			virtual RenderGroup *createGroup(const Shape *shape) = 0;
			virtual bool destroyGroup(RenderGroup *group) = 0;
	};
}

#endif // !GPWE_RENDERER_HPP

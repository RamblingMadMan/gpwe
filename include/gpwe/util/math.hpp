#ifndef GPWE_MATH_HPP
#define GPWE_MATH_HPP 1

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

#include "glm/mat2x2.hpp"
#include "glm/mat3x3.hpp"
#include "glm/mat4x4.hpp"

namespace gpwe{
	using Vec2 = glm::vec2;
	using Vec3 = glm::vec3;
	using Vec4 = glm::vec4;

	using Mat2 = glm::mat2;
	using Mat3 = glm::mat3;
	using Mat4 = glm::mat4;

	struct AABB{
		Vec3 min, max;
	};
}

#endif // !GPWE_MATH_HPP

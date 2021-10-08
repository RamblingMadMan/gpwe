#ifndef GPWE_SHAPE_HPP
#define GPWE_SHAPE_HPP 1

#include <cstdint>

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

namespace gpwe{
	class Shape{
		public:
			enum class Kind{
				points,
				lines,
				tris,
				triStrip,
				triFan,

				count
			};

			virtual ~Shape() = default;

			virtual Kind kind() const noexcept = 0;

			virtual std::uint32_t numPoints() const noexcept = 0;
			virtual const glm::vec3 *vertices() const noexcept = 0;
			virtual const glm::vec3 *normals() const noexcept = 0;
			virtual const glm::vec2 *uvs(std::uint8_t channel = 0) const noexcept = 0;

			virtual std::uint32_t numIndices() const noexcept = 0;
			virtual const std::uint32_t *indices() const noexcept = 0;
	};

	namespace shapes{
		class Quad: public Shape{
			public:
				Quad(
					const glm::vec3 &tl, const glm::vec3 &tr,
					const glm::vec3 &bl, const glm::vec3 &br
				);

				Kind kind() const noexcept override{ return Kind::triFan; }

				std::uint32_t numPoints() const noexcept override{ return 4; }
				const glm::vec3 *vertices() const noexcept override{ return m_verts; }
				const glm::vec3 *normals() const noexcept override{ return m_norms; }
				const glm::vec2 *uvs(std::uint8_t channel = 0) const noexcept override{ return m_uvs; }

				std::uint32_t numIndices() const noexcept override{ return 4; }

				const std::uint32_t *indices() const noexcept override{
					static constexpr std::uint32_t arr[] = { 0, 1, 2, 3 };
					return arr;
				}

			private:
				glm::vec3 m_verts[4];
				glm::vec3 m_norms[4];
				glm::vec2 m_uvs[4];
		};

		class Rect: public Quad{
			public:
				Rect(float w, float h);
		};

		class Square: public Rect{
			public:
				Square(float d);
		};
	}
}

#endif // !GPWE_SHAPE_HPP

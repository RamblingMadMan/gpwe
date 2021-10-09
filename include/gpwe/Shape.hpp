#ifndef GPWE_SHAPE_HPP
#define GPWE_SHAPE_HPP 1

#include <cstdint>
#include <array>
#include <vector>

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

				Kind kind() const noexcept override{ return Kind::tris; }

				std::uint32_t numPoints() const noexcept override{ return 4; }

				const glm::vec3 *vertices() const noexcept override{ return m_verts; }

				const glm::vec3 *normals() const noexcept override{ return m_norms; }

				const glm::vec2 *uvs(std::uint8_t channel = 0) const noexcept override{
					static constexpr glm::vec2 arr[] = {
						{ 0.f, 0.f }, { 1.f, 0.f },
						{ 1.f, 1.f }, { 0.f, 1.f }
					};

					return arr;
				}

				std::uint32_t numIndices() const noexcept override{ return 6; }

				const std::uint32_t *indices() const noexcept override{
					static constexpr std::uint32_t arr[] = { 0, 1, 2, 0, 2, 3 };

					return arr;
				}

			private:
				glm::vec3 m_verts[4], m_norms[4];
		};

		class Rect: public Quad{
			public:
				Rect(float w, float h);
		};

		class Square: public Rect{
			public:
				Square(float d): Rect(d, d){}
		};

		class Hexahedron: public Shape{
			public:
				Hexahedron(
					// front
					const glm::vec3 &ftl, const glm::vec3 &ftr,
					const glm::vec3 &fbl, const glm::vec3 &fbr,

					// back
					const glm::vec3 &btl, const glm::vec3 &btr,
					const glm::vec3 &bbl, const glm::vec3 &bbr
				);

				Kind kind() const noexcept override{ return Kind::tris; }

				std::uint32_t numPoints() const noexcept override{ return std::size(m_verts); }

				const glm::vec3 *vertices() const noexcept override{ return m_verts; }
				const glm::vec3 *normals() const noexcept override{ return m_norms; }
				const glm::vec2 *uvs(std::uint8_t channel = 0) const noexcept override;

				std::uint32_t numIndices() const noexcept override{ return 36; }

				const std::uint32_t *indices() const noexcept override;

			private:
				glm::vec3 m_verts[24], m_norms[24];
		};

		class Cuboid: public Hexahedron{
			public:
				Cuboid(float w, float h, float d)
					: Cuboid(Half, std::abs(w * 0.5f), std::abs(h * 0.5f), std::abs(d * 0.5f))
				{}

			private:
				struct HalfT{} static Half;

				Cuboid(HalfT, float hw, float hh, float hd);
		};

		class Cube: public Cuboid{
			public:
				Cube(float d): Cuboid(d, d, d){}
		};

		class TriangleMesh: public Shape{
			public:
				TriangleMesh(
					std::vector<glm::vec3> verts_,
					std::vector<glm::vec3> norms_,
					std::vector<glm::vec2> uvs_,
					std::vector<std::uint32_t> indices_
				)
					: m_verts(std::move(verts_))
					, m_norms(std::move(norms_))
					, m_uvs(std::move(uvs_))
					, m_indices(std::move(indices_))
				{
					assert(m_verts.size() == m_norms.size() && m_verts.size() == m_uvs.size());
				}

				Kind kind() const noexcept override{ return Kind::tris; }

				std::uint32_t numPoints() const noexcept override{ return m_verts.size(); }

				const glm::vec3 *vertices() const noexcept override{ return m_verts.data(); }
				const glm::vec3 *normals() const noexcept override{ return m_norms.data(); }
				const glm::vec2 *uvs(std::uint8_t channel = 0) const noexcept override{ return m_uvs.data(); }

				std::uint32_t numIndices() const noexcept override{ return m_indices.size(); }

				const std::uint32_t *indices() const noexcept override{ return m_indices.data(); }

			private:
				std::vector<glm::vec3> m_verts, m_norms;
				std::vector<glm::vec2> m_uvs;
				std::vector<std::uint32_t> m_indices;
		};
	}
}

#endif // !GPWE_SHAPE_HPP

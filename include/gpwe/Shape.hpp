#ifndef GPWE_SHAPE_HPP
#define GPWE_SHAPE_HPP 1

#include <cstdint>
#include <array>

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

#include "Vector.hpp"

namespace gpwe{
	class Shape{
		public:
			enum class Kind{
				vertex, heightmap,

				count
			};

			virtual ~Shape() = default;

			virtual Kind kind() const noexcept = 0;
	};

	class VertexShape: public Shape{
		public:
			enum class Mode{
				points,
				lines,
				tris,
				triStrip,
				triFan,

				count
			};

			Kind kind() const noexcept override{ return Kind::vertex; }

			virtual Mode mode() const noexcept = 0;

			virtual std::uint32_t numPoints() const noexcept = 0;
			virtual const glm::vec3 *vertices() const noexcept = 0;
			virtual const glm::vec3 *normals() const noexcept = 0;
			virtual const glm::vec2 *uvs(std::uint8_t channel = 0) const noexcept = 0;

			virtual std::uint32_t numIndices() const noexcept = 0;
			virtual const std::uint32_t *indices() const noexcept = 0;
	};

	namespace shapes{
		class Quad: public VertexShape{
			public:
				Quad(
					const glm::vec3 &tl, const glm::vec3 &tr,
					const glm::vec3 &bl, const glm::vec3 &br
				);

				Mode mode() const noexcept override{ return Mode::tris; }

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

		class Hexahedron: public VertexShape{
			public:
				Hexahedron(
					// front
					const glm::vec3 &ftl, const glm::vec3 &ftr,
					const glm::vec3 &fbl, const glm::vec3 &fbr,

					// back
					const glm::vec3 &btl, const glm::vec3 &btr,
					const glm::vec3 &bbl, const glm::vec3 &bbr
				);

				Mode mode() const noexcept override{ return Mode::tris; }

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

		class TriangleMesh: public VertexShape{
			public:
				TriangleMesh(
					Vector<glm::vec3> verts_,
					Vector<glm::vec3> norms_,
					Vector<glm::vec2> uvs_,
					Vector<std::uint32_t> indices_
				)
					: m_verts(std::move(verts_))
					, m_norms(std::move(norms_))
					, m_uvs(std::move(uvs_))
					, m_indices(std::move(indices_))
				{
					assert(m_verts.size() == m_norms.size() && m_verts.size() == m_uvs.size());
				}

				Mode mode() const noexcept override{ return Mode::tris; }

				std::uint32_t numPoints() const noexcept override{ return m_verts.size(); }

				const glm::vec3 *vertices() const noexcept override{ return m_verts.data(); }
				const glm::vec3 *normals() const noexcept override{ return m_norms.data(); }
				const glm::vec2 *uvs(std::uint8_t channel = 0) const noexcept override{ return m_uvs.data(); }

				std::uint32_t numIndices() const noexcept override{ return m_indices.size(); }

				const std::uint32_t *indices() const noexcept override{ return m_indices.data(); }

			private:
				Vector<glm::vec3> m_verts, m_norms;
				Vector<glm::vec2> m_uvs;
				Vector<std::uint32_t> m_indices;
		};
	}

	class HeightMapShape: public Shape{
		public:
			HeightMapShape(std::uint32_t w, std::uint32_t h, const float *vals) noexcept
				: m_w(w), m_h(h), m_values(vals, vals + (w * h)){}

			Kind kind() const noexcept override{ return Kind::heightmap; }

			std::uint32_t width() const noexcept{ return m_w; }
			std::uint32_t height() const noexcept{ return m_h; }
			const float *values() const noexcept{ return m_values.data(); }

			shapes::TriangleMesh generateMesh(float scale = 1.f) const;

		private:
			std::uint32_t m_w, m_h;
			Vector<float> m_values;
	};
}

#endif // !GPWE_SHAPE_HPP

#ifndef GPWE_SHAPE_HPP
#define GPWE_SHAPE_HPP 1

#include <cstdint>
#include <array>

#include "util/Object.hpp"
#include "util/Vector.hpp"
#include "util/math.hpp"
#include "log.hpp"

namespace gpwe{
	enum class ShapeKind{
		vertex, heightmap,

		count
	};

	class Shape{
		public:
			using Kind = ShapeKind;

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

			virtual Nat32 numPoints() const noexcept = 0;
			virtual const Vec3 *vertices() const noexcept = 0;
			virtual const Vec3 *normals() const noexcept = 0;
			virtual const Vec2 *uvs(Nat8 channel = 0) const noexcept = 0;

			virtual Nat32 numIndices() const noexcept = 0;
			virtual const Nat32 *indices() const noexcept = 0;
	};

	namespace shapes{
		class Quad: public VertexShape{
			public:
				Quad(
					const Vec3 &tl, const Vec3 &tr,
					const Vec3 &bl, const Vec3 &br
				);

				Mode mode() const noexcept override{ return Mode::tris; }

				Nat32 numPoints() const noexcept override{ return 4; }

				const Vec3 *vertices() const noexcept override{ return m_verts; }

				const Vec3 *normals() const noexcept override{ return m_norms; }

				const Vec2 *uvs(Nat8 channel = 0) const noexcept override{
					static constexpr Vec2 arr[] = {
						{ 0.f, 0.f }, { 1.f, 0.f },
						{ 1.f, 1.f }, { 0.f, 1.f }
					};

					return arr;
				}

				Nat32 numIndices() const noexcept override{ return 6; }

				const Nat32 *indices() const noexcept override{
					static constexpr Nat32 arr[] = { 0, 1, 2, 0, 2, 3 };

					return arr;
				}

			private:
				Vec3 m_verts[4], m_norms[4];
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
					const Vec3 &ftl, const Vec3 &ftr,
					const Vec3 &fbl, const Vec3 &fbr,

					// back
					const Vec3 &btl, const Vec3 &btr,
					const Vec3 &bbl, const Vec3 &bbr
				);

				Mode mode() const noexcept override{ return Mode::tris; }

				Nat32 numPoints() const noexcept override{ return std::size(m_verts); }

				const Vec3 *vertices() const noexcept override{ return m_verts; }
				const Vec3 *normals() const noexcept override{ return m_norms; }
				const Vec2 *uvs(Nat8 channel = 0) const noexcept override;

				Nat32 numIndices() const noexcept override{ return 36; }

				const Nat32 *indices() const noexcept override;

			private:
				Vec3 m_verts[24], m_norms[24];
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
					Vector<Vec3> verts_,
					Vector<Vec3> norms_,
					Vector<Vec2> uvs_,
					Vector<Nat32> indices_
				)
					: m_verts(std::move(verts_))
					, m_norms(std::move(norms_))
					, m_uvs(std::move(uvs_))
					, m_indices(std::move(indices_))
				{
					assert(m_verts.size() == m_norms.size() && m_verts.size() == m_uvs.size());
				}

				Mode mode() const noexcept override{ return Mode::tris; }

				Nat32 numPoints() const noexcept override{ return m_verts.size(); }

				const Vec3 *vertices() const noexcept override{ return m_verts.data(); }
				const Vec3 *normals() const noexcept override{ return m_norms.data(); }
				const Vec2 *uvs(Nat8 channel = 0) const noexcept override{ return m_uvs.data(); }

				Nat32 numIndices() const noexcept override{ return m_indices.size(); }

				const Nat32 *indices() const noexcept override{ return m_indices.data(); }

			private:
				Vector<Vec3> m_verts, m_norms;
				Vector<Vec2> m_uvs;
				Vector<Nat32> m_indices;
		};
	}

	class HeightMapShape: public Shape{
		public:
			HeightMapShape(Nat16 w, Nat16 h, Vector<float> values_) noexcept
				: m_w(w), m_h(h), m_values(std::move(values_))
			{
				if(m_values.size() == 0){
					m_values.resize(w*h, 0.f);
				}
				else if(m_values.size() != w * h){
					log::errorLn(
						"invalid size {}x{}={} given for vector of size {}",
						w, h, w * h, m_values.size()
					);

					m_values.clear();
					m_values.resize(w*h, 0.f);
				}
			}

			static HeightMapShape createSimpleTerrain(Nat16 resolution = 256);

			Kind kind() const noexcept override{ return Kind::heightmap; }

			Nat16 width() const noexcept{ return m_w; }
			Nat16 height() const noexcept{ return m_h; }
			const float *values() const noexcept{ return m_values.data(); }

			shapes::TriangleMesh generateMesh(float xyScale = 100.f, float maxHeight = 1.f) const;

		private:
			Nat16 m_w, m_h;
			Vector<float> m_values;
	};
}

#endif // !GPWE_SHAPE_HPP

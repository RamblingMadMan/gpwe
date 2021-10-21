#ifndef GPWE_CAMERA_HPP
#define GPWE_CAMERA_HPP 1

#include "util/math.hpp"

namespace gpwe{
	class Camera{
		public:
			Camera(float fovy, float aspect, float nearz = 0.001f, float farz = 1000.f);

			void setProj(float fovy, float aspect, float nearz = 0.001f, float farz = 1000.f);

			void setPosition(const Vec3 &p) noexcept{
				m_pos = p;
				m_isDirty = true;
			}

			void translate(const Vec3 &v) noexcept{
				m_pos += v;
				m_isDirty = true;
			}

			void rotate(float rads, const Vec3 &axis) noexcept;

			const Vec3 &pos() const noexcept{ return m_pos; }
			const Vec3 &pyr() const noexcept{ return m_pyr; }

			const Vec3 &forward() const noexcept{ viewMat(); return m_forward; }
			const Vec3 &right() const noexcept{ viewMat(); return m_right; }
			const Vec3 &up() const noexcept{ viewMat(); return m_up; }

			const Mat4 &projMat() const noexcept{ return m_proj; }

			Mat4 viewMat() const noexcept;

		private:
			Vec3 m_pos, m_pyr;
			Mat4 m_proj;

			mutable Mat4 m_view;
			mutable Vec3 m_forward, m_right, m_up;
			mutable bool m_isDirty = true;
	};
}

#endif // !GPWE_CAMERA_HPP

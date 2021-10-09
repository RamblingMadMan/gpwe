#ifndef GPWE_CAMERA_HPP
#define GPWE_CAMERA_HPP 1

#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"

namespace gpwe{
	class Camera{
		public:
			Camera(float fovy, float aspect, float nearz = 0.001f, float farz = 1000.f);

			void setProj(float fovy, float aspect, float nearz = 0.001f, float farz = 1000.f);

			void setPosition(const glm::vec3 &p) noexcept{
				m_pos = p;
				m_isDirty = true;
			}

			void translate(const glm::vec3 &v) noexcept{
				m_pos += v;
				m_isDirty = true;
			}

			void rotate(float rads, const glm::vec3 &axis) noexcept;

			const glm::vec3 &pos() const noexcept{ return m_pos; }
			const glm::vec3 &pyr() const noexcept{ return m_pyr; }

			const glm::vec3 &forward() const noexcept{ viewMat(); return m_forward; }
			const glm::vec3 &right() const noexcept{ viewMat(); return m_right; }
			const glm::vec3 &up() const noexcept{ viewMat(); return m_up; }

			const glm::mat4 &projMat() const noexcept{ return m_proj; }

			glm::mat4 viewMat() const noexcept;

		private:
			glm::vec3 m_pos, m_pyr;
			glm::mat4 m_proj;

			mutable glm::mat4 m_view;
			mutable glm::vec3 m_forward, m_right, m_up;
			mutable bool m_isDirty = true;
	};
}

#endif // !GPWE_CAMERA_HPP

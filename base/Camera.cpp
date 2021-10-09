#define GLM_FORCE_RADIANS
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/rotate_vector.hpp"

#include "gpwe/Camera.hpp"

using namespace gpwe;

Camera::Camera(float fovy, float aspect, float nearz, float farz)
	: m_pos(0.f, 0.f, 0.f)
	, m_pyr(0.f, M_PI, 0.f)
{
	setProj(fovy, aspect, nearz, farz);
}

void Camera::setProj(float fovy, float aspect, float nearz, float farz){
	m_proj = glm::perspectiveLH_ZO(fovy, aspect, nearz, farz);
}

void Camera::rotate(float rads, const glm::vec3 &axis) noexcept{
	m_pyr += axis * rads;

	auto pitchCenter = 0.f;
	auto pitchLimit = float(M_PI_2 - 0.1);
	auto pitchUpper = pitchCenter + pitchLimit;
	auto pitchLower = pitchCenter - pitchLimit;

	if(m_pyr.x > pitchUpper){
		m_pyr.x = pitchUpper;
	}
	else if(m_pyr.x < pitchLower){
		m_pyr.x = pitchLower;
	}

	m_pyr.y = std::fmod(m_pyr.y, float(M_PI * 2.0));
	m_pyr.z = std::fmod(m_pyr.z, float(M_PI * 2.0));
	m_isDirty = true;
}

glm::mat4 Camera::viewMat() const noexcept{
	if(m_isDirty){
		auto pitch = glm::angleAxis(m_pyr.x, glm::vec3(1.f, 0.f, 0.f));
		auto yaw = glm::angleAxis(m_pyr.y, glm::vec3(0.f, 1.f, 0.f));
		auto roll = glm::angleAxis(m_pyr.z, glm::vec3(0.f, 0.f, 1.f));

		auto orientation = glm::normalize(pitch * yaw * roll);

		m_forward = orientation * glm::vec3(0.f, 0.f, -1.f);
		m_right = orientation * glm::vec3(1.f, 0.f, 0.f);
		m_up = orientation * glm::vec3(0.f, -1.f, 0.f);

		auto rot = glm::mat4_cast(glm::quatLookAtLH(m_forward, m_up));
		auto trans = glm::translate(glm::mat4(1.0f), -m_pos);

		m_view = rot * trans;
		m_isDirty = false;
	}

	return m_view;
}

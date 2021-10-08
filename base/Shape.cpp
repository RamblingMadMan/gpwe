#include "glm/gtx/normal.hpp"

#include "gpwe/Shape.hpp"

using namespace gpwe;

shapes::Quad::Quad(
	const glm::vec3 &tl, const glm::vec3 &tr,
	const glm::vec3 &bl, const glm::vec3 &br
)
	: m_verts{ bl, br, tr, tl }
{
	auto normA = glm::triangleNormal(bl, br, tr);
	auto normB = glm::triangleNormal(bl, tr, tl);
	auto normAvg = glm::normalize((normA + normB) / 2.f);

	m_norms[0] = normAvg;
	m_norms[1] = normAvg;
	m_norms[2] = normAvg;
	m_norms[3] = normAvg;

	m_uvs[0] = glm::vec2(0.f, 0.f);
	m_uvs[0] = glm::vec2(1.f, 0.f);
	m_uvs[0] = glm::vec2(1.f, 1.f);
	m_uvs[0] = glm::vec2(0.f, 1.f);
}

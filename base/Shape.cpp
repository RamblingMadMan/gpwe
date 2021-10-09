#include <array>

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
}

shapes::Rect::Rect(float w, float h)
	: Quad{
			{ -w * 0.5f,  h * 0.5f, 0.f },
			{  w * 0.5f,  h * 0.5f, 0.f },
			{ -w * 0.5f, -h * 0.5f, 0.f },
			{  w * 0.5f, -h * 0.5f, 0.f },
		}
{}

shapes::Hexahedron::Hexahedron(
	// front
	const glm::vec3 &ftl, const glm::vec3 &ftr,
	const glm::vec3 &fbl, const glm::vec3 &fbr,

	// back
	const glm::vec3 &btl, const glm::vec3 &btr,
	const glm::vec3 &bbl, const glm::vec3 &bbr
)
	: m_verts{
			fbl, fbr, ftr, ftl, // front
			bbr, bbl, btl, btr, // back

			ftl, ftr, btr, btl, // top
			bbl, bbr, fbr, fbl, // bottom

			bbl, fbl, ftl, btl, // left
			fbr, bbr, btr, ftr // right
		}
{
	// for all faces calculate normal
	for(int i = 0; i < 6; i++){
		auto verts = m_verts + (i * 4);
		auto norms = m_norms + (i * 4);

		auto n0 = glm::triangleNormal(verts[0], verts[1], verts[2]);
		auto n1 = glm::triangleNormal(verts[0], verts[2], verts[3]);
		auto nAvg = glm::normalize((n0 + n1) / 2.f);

		for(int j = 0; j < 4; j++){
			norms[j] = nAvg;
		}
	}
}

const glm::vec2 *shapes::Hexahedron::uvs(std::uint8_t channel) const noexcept{
	static constexpr glm::vec2 arr[][24] = {
		{
			// Same uvs for all faces (6 * the same uvs)

			{ 0.f, 0.f }, { 1.f, 0.f },
			{ 1.f, 1.f }, { 0.f, 1.f },

			{ 0.f, 0.f }, { 1.f, 0.f },
			{ 1.f, 1.f }, { 0.f, 1.f },

			{ 0.f, 0.f }, { 1.f, 0.f },
			{ 1.f, 1.f }, { 0.f, 1.f },

			{ 0.f, 0.f }, { 1.f, 0.f },
			{ 1.f, 1.f }, { 0.f, 1.f },

			{ 0.f, 0.f }, { 1.f, 0.f },
			{ 1.f, 1.f }, { 0.f, 1.f },

			{ 0.f, 0.f }, { 1.f, 0.f },
			{ 1.f, 1.f }, { 0.f, 1.f }
		}
	};

	return arr[std::min((std::size_t)channel, std::size(arr)-1)];
}

const std::uint32_t *shapes::Hexahedron::indices() const noexcept{
	static constexpr std::uint32_t arr[36] = {
		// front
		0, 1, 2,
		0, 2, 3,

		// back
		4, 5, 6,
		4, 6, 7,

		// top
		8, 9, 10,
		8, 10, 11,

		// bottom
		12, 13, 14,
		12, 14, 15,

		// left
		16, 17, 18,
		16, 18, 19,

		// right
		20, 21, 22,
		20, 22, 23
	};

	return arr;
}
shapes::Cuboid::Cuboid(HalfT, float hw, float hh, float hd)
	: Hexahedron(
			// points in code visual order
			// front
			{ -hw,  hh, -hd }, {  hw,  hh, -hd },
			{ -hw, -hh, -hd }, {  hw, -hh, -hd },

			// back
			{ -hw,  hh, hd }, {  hw,  hh, hd },
			{ -hw, -hh, hd }, {  hw, -hh, hd }
		)
{}

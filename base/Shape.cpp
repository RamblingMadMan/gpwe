#include <array>

#include "FastNoise/FastNoise.h"

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

		auto n0 = glm::triangleNormal(verts[0], verts[2], verts[1]);
		auto n1 = glm::triangleNormal(verts[0], verts[3], verts[2]);
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

HeightMapShape HeightMapShape::createSimpleTerrain(std::uint16_t resolution, float scale){
	// straight ripped from the FastNoise2 library tests
	// https://github.com/Auburn/FastNoise2/blob/master/tests/FastNoiseCpp11Include.cpp
	auto noiseNode = FastNoise::New<FastNoise::FractalFBm>();

	noiseNode->SetSource(FastNoise::New<FastNoise::Simplex>());
	noiseNode->SetGain(FastNoise::New<FastNoise::Value>());

	Vector<float> heights;
	heights.resize(resolution * resolution);

	noiseNode->GenUniformGrid2D(heights.data(), 0, 0, resolution, resolution, 0.02f, 1337);

	std::transform(
		begin(heights), end(heights),
		begin(heights),
		[scale](float y){ return y * scale; }
	);

	return HeightMapShape(resolution, resolution, std::move(heights));
}

shapes::TriangleMesh HeightMapShape::generateMesh(float scale) const{
	const float aspect = m_w > m_h
						 ? float(m_w) / float(m_h)
						 : float(m_h) / float(m_w);

	const float dimX = m_w > m_h
					   ? m_w * scale * aspect
					   : m_w * scale;

	const float dimY = m_w > m_h
					   ? m_h * scale
					   : m_h * scale * aspect;

	std::uint32_t numPoints = m_w * m_h;
	std::uint32_t numIndices = ((m_w-1) * (m_h-1)) * 2 * 3;
	Vector<glm::vec3> verts, norms;
	Vector<glm::vec2> uvs;
	Vector<std::uint32_t> indices;

	verts.reserve(numPoints);
	norms.reserve(numPoints);
	uvs.reserve(numPoints);
	indices.reserve(numIndices);

	const float yStep = 1.f / m_h;
	const float xStep = 1.f / m_w;

	for(std::uint16_t y = 0; y < m_h; y++){
		const std::uint16_t yIdx = y * m_w;
		const float yRel = y * yStep;
		const float yOff = yRel - 0.5f;
		for(std::uint16_t x = 0; x < m_w; x++){
			const std::uint16_t idx = yIdx + x;
			const float xRel = x * xStep;
			const float xOff = xRel - 0.5f;
			verts.emplace_back(xOff * dimX, m_values[idx], yOff * dimY);
			norms.emplace_back(glm::vec3(0.f, 0.f, 0.f)); // Zeroed for face average
			uvs.emplace_back(glm::vec2(xRel, 1.f - yRel));
		}
	}

	struct Tri{
		std::uint32_t indices[3];
		glm::vec3 normal;
	};

	Vector<Tri> tris;
	tris.reserve(m_w * m_h * 3);

	std::uint32_t y0Idx = 0;

	for(std::uint16_t y = 1; y < m_h; y++){
		const std::uint32_t y1Idx = y * m_w;

		for(std::uint16_t x = 0; x < (m_w - 1); x++){
			std::uint32_t i0 = y0Idx + x;
			std::uint32_t i1 = y0Idx + x + 1;
			std::uint32_t i2 = y1Idx + x + 1;
			std::uint32_t i3 = y1Idx + x;

			const auto &p0 = verts[i0];
			const auto &p1 = verts[i1];
			const auto &p2 = verts[i2];
			const auto &p3 = verts[i3];

			Tri f0{ .indices = { i0, i1, i2 }, .normal = glm::triangleNormal(p2, p1, p0) };
			Tri f1{ .indices = { i0, i2, i3 }, .normal = glm::triangleNormal(p3, p2, p0) };

			tris.push_back(f0);
			tris.push_back(f1);
		}

		y0Idx = y1Idx;
	}

	for(auto &&tri : tris){
		for(auto idx : tri.indices){
			indices.emplace_back(idx);
			norms[idx] += tri.normal;
		}
	}

	for(auto &&norm : norms){
		norm = glm::normalize(norm);
	}

	return shapes::TriangleMesh(std::move(verts), std::move(norms), std::move(uvs), std::move(indices));
}

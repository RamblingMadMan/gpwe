#include <array>

#include "FastNoise/FastNoise.h"

#include "glm/gtx/normal.hpp"

#include "gpwe/Shape.hpp"

using namespace gpwe;

shapes::Quad::Quad(
	const Vec3 &tl, const Vec3 &tr,
	const Vec3 &bl, const Vec3 &br
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
	const Vec3 &ftl, const Vec3 &ftr,
	const Vec3 &fbl, const Vec3 &fbr,

	// back
	const Vec3 &btl, const Vec3 &btr,
	const Vec3 &bbl, const Vec3 &bbr
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

const Nat32 *shapes::Hexahedron::indices() const noexcept{
	static constexpr Nat32 arr[36] = {
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

HeightMapShape HeightMapShape::createSimpleTerrain(Nat16 resolution){
	// straight ripped from the FastNoise2 library tests
	// https://github.com/Auburn/FastNoise2/blob/master/tests/FastNoiseCpp11Include.cpp
	auto noiseNode = FastNoise::New<FastNoise::FractalFBm>();

	noiseNode->SetSource(FastNoise::New<FastNoise::Simplex>());
	noiseNode->SetGain(FastNoise::New<FastNoise::Value>());

	Vector<float> heights;
	heights.resize(resolution * resolution);

	noiseNode->GenUniformGrid2D(heights.data(), 0, 0, resolution, resolution, 0.02f, 1337);

	Vector<float> heightNoise;
	heightNoise.resize(resolution * resolution);

	noiseNode->GenUniformGrid2D(heightNoise.data(), 0, 0, resolution, resolution, 1.f / 3.f, 1337);

	Vector<float> finalHeights;
	finalHeights.resize(resolution * resolution);

	std::transform(
		heights.begin(), heights.end(),
		heightNoise.begin(),
		finalHeights.begin(),
		[](auto h, auto n){ return h - (n * 0.01f); }
	);

	return HeightMapShape(resolution, resolution, std::move(finalHeights));
}

shapes::TriangleMesh HeightMapShape::generateMesh(float scale, float maxHeight) const{
	const float aspect = float(m_w) / float(m_h);

	const float dimX = m_w * scale;
	const float dimY = m_h * scale * aspect;

	Nat32 numPoints = m_w * m_h;
	Nat32 numIndices = ((m_w-1) * (m_h-1)) * 2 * 3;
	Vector<Vec3> verts;
	Vector<Vec4> norms;
	Vector<glm::vec2> uvs;
	Vector<Nat32> indices;

	verts.reserve(numPoints);
	norms.reserve(numPoints);
	uvs.reserve(numPoints);
	indices.reserve(numIndices);

	const float yStep = 1.f / m_h;
	const float xStep = 1.f / m_w;

	const float xyOff = scale * -0.5f;

	for(Nat16 y = 0; y < m_h; y++){
		const Nat16 yIdx = y * m_w;
		const float yRel = y * yStep;
		for(Nat16 x = 0; x < m_w; x++){
			const Nat16 idx = yIdx + x;
			const float xRel = x * xStep;

			verts.emplace_back(xyOff + (xRel * scale), m_values[idx] * maxHeight, xyOff + (yRel * scale));
			norms.emplace_back(Vec4(0.f)); // Zeroed for face average
			uvs.emplace_back(glm::vec2(xRel, 1.f - yRel));
		}
	}

	Nat32 y0Idx = 0;

	for(Nat16 y = 1; y < m_h; y++){
		const Nat32 y1Idx = y * m_w;

		for(Nat16 x = 0; x < (m_w - 1); x++){
			Nat32 i0 = y0Idx + x;
			Nat32 i1 = y0Idx + x + 1;
			Nat32 i2 = y1Idx + x + 1;
			Nat32 i3 = y1Idx + x;

			const auto normA = glm::triangleNormal(verts[i2], verts[i1], verts[i0]);
			const auto normB = glm::triangleNormal(verts[i3], verts[i2], verts[i0]);

			norms[i0] += Vec4(normA + normB, 2.f);
			norms[i1] += Vec4(normA, 1.f);
			norms[i2] += Vec4(normA + normB, 2.f);
			norms[i3] += Vec4(normB, 1.f);

			indices.emplace_back(i0);
			indices.emplace_back(i1);
			indices.emplace_back(i2);

			indices.emplace_back(i0);
			indices.emplace_back(i2);
			indices.emplace_back(i3);
		}

		y0Idx = y1Idx;
	}

	for(auto &norm : norms){
		norm /= std::max(norm.w, 1.f);
		norm = Vec4(glm::normalize(Vec3(norm)), 1.f);
	}

	auto avgNormals = Vector<Vec3>(norms.begin(), norms.end());

	/*
	// Calculate inner normals
	for(Nat16 y = 1; y < (m_h - 1); y++){
		for(Nat16 x = 1; x < (m_w - 1); x++){
			Nat16 idx = (y * m_w) + x;

			const Vec3 &p = verts[idx];
			const Vec3 &above = verts[idx - m_w];
			const Vec3 &below = verts[idx + m_w];
			const Vec3 &left = verts[idx - 1];
			const Vec3 &right = verts[idx + 1];

			Vec4 &norm = norms[idx];
			norm += Vec4(glm::triangleNormal(above, right, p), 1.f);
			norm += Vec4(glm::triangleNormal(right, below, p), 1.f);
			norm += Vec4(glm::triangleNormal(below, left, p), 1.f);
			norm += Vec4(glm::triangleNormal(left, above, p), 1.f);
		}
	}

	// Average normals
	for(Nat16 y = 1; y < (m_h - 1); y++){
		for(Nat16 x = 1; x < (m_w - 1); x++){

		}
	}
	*/

	return shapes::TriangleMesh(std::move(verts), std::move(avgNormals), std::move(uvs), std::move(indices));
}

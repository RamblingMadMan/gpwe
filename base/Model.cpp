#include <algorithm>

#include "gpwe/log.hpp"
#include "gpwe/Model.hpp"

#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/Importer.hpp"

using namespace gpwe;

Model::Model(const fs::path &filePath){
	loadModel(filePath);
}

void Model::loadModel(const fs::path &filePath){
	Assimp::Importer importer;

	auto scene = importer.ReadFile(
		filePath.string(),
		aiProcess_GenSmoothNormals | aiProcess_GenUVCoords | aiProcess_Triangulate |
		aiProcess_SortByPType | aiProcess_MakeLeftHanded | aiProcess_ImproveCacheLocality |
		aiProcess_OptimizeMeshes
	);

	if(!scene){
		logErrorLn("Could not import {}: {}", filePath.string(), importer.GetErrorString());
		return;
	}

	m_meshes.clear();
	m_meshes.reserve(scene->mNumMeshes);

	for(std::uint32_t i = 0; i < scene->mNumMeshes; i++){
		auto mesh = scene->mMeshes[i];

		if(mesh->mNumUVComponents[0] == 0){
			logErrorLn("Could not import mesh '{}' from '{}'", mesh->mName.C_Str(), filePath.string());
			continue;
		}

		Vector<glm::vec3> verts, norms;
		Vector<glm::vec2> uvs;

		verts.reserve(mesh->mNumVertices);
		norms.reserve(mesh->mNumVertices);
		uvs.reserve(mesh->mNumVertices);

		for(std::uint32_t j = 0; j < mesh->mNumVertices; j++){
			auto vert = mesh->mVertices + j;
			auto norm = mesh->mNormals + j;
			auto uv = mesh->mTextureCoords[0] + j;
			verts.emplace_back(vert->x, vert->y, vert->z);
			norms.emplace_back(norm->y, norm->y, norm->z);
			uvs.emplace_back(uv->x, uv->y);
		}

		Vector<std::uint32_t> indices;

		indices.reserve(mesh->mNumFaces * 3);

		for(std::uint32_t j = 0; j < mesh->mNumFaces; j++){
			auto face = mesh->mFaces + j;
			indices.emplace_back(face->mIndices[0]);
			indices.emplace_back(face->mIndices[1]);
			indices.emplace_back(face->mIndices[2]);
		}

		m_meshes.emplace_back(std::move(verts), std::move(norms), std::move(uvs), std::move(indices));
	}
}

Vector<const VertexShape*> Model::shapes() const{
	Vector<const VertexShape*> ret;
	ret.reserve(m_meshes.size());
	std::transform(
		m_meshes.begin(), m_meshes.end(),
		std::back_inserter(ret),
		[](const auto &it){ return &it; }
	);
	return ret;
}

#include "gpwe/log.hpp"
#include "gpwe/resource.hpp"

#include "physfs.h"

#include "magic.h"

#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/Importer.hpp"

extern magic_t gpweMagic;

namespace gpwe::resource{
	class BinaryFile: public Asset{
		public:
			BinaryFile(Access access_, Str path_, Vector<char> bytes)
				: Asset(Kind::file, access_, Category::binary, std::move(path_))
				, m_bytes(std::move(bytes))
			{
				setData(Kind::file, access_, m_bytes.size(), { .w = m_bytes.data() });
			}

		private:
			Vector<char> m_bytes;
			PHYSFS_sint64 m_createTime = 0, m_modTime = 0;
			bool m_hasChanges = false;
	};

	class ModelFile: public resource::Model{
		public:
			ModelFile(Access access_, Str path_, Vector<shapes::TriangleMesh> meshes_)
				: Model(Kind::file, access_, std::move(path_), std::move(meshes_)){}
	};
}

namespace gpwe{
	class PhysFSError: public std::exception{
		public:
			PhysFSError(): m_msg(PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())){}

			virtual const char *what(){ return m_msg.c_str(); }

		private:
			Str m_msg;
	};
}

using namespace gpwe;

resource::Manager::Manager(){}

resource::Manager::~Manager(){
}

bool resource::Manager::mount(const fs::path &path, StrView dir, bool mountBefore){
	if(!fs::exists(path)){
		return false;
	}

	auto pathStr = path.string<char, std::char_traits<char>, Allocator<char>>(Allocator<char>{});
	auto dirStr = Str(dir);
	auto res = PHYSFS_mount(pathStr.c_str(), dirStr.c_str(), !mountBefore);
	if(!res){
		logErrorLn("Error in PHYSFS_mount: {}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
	}

	return res != 0;
}

resource::Asset *resource::Manager::openFile(StrView path, Asset::Access access_){
	if(access_ == Asset::Access::write){
		access_ = Asset::Access::readWrite;
	}

	auto mapRes = m_files.find(path);

	if(mapRes != m_files.end()){
		auto ret = mapRes->second;
		auto fileFlags = (std::uint8_t)ret->access();
		auto openFlags = (std::uint8_t)access_;
		if((fileFlags & openFlags) != openFlags){
			logErrorLn("Can not open file with access flags 0x{:h}", openFlags);
			return nullptr;
		}

		return ret;
	}

	auto pathStr = Str(path);

	auto getPhysFSError = []{ return PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()); };

	if(!PHYSFS_exists(pathStr.c_str())){
		logErrorLn("File does not exist: {}", pathStr);
		return nullptr;
	}

	PHYSFS_Stat info;

	if(!PHYSFS_stat(pathStr.c_str(), &info)){
		logErrorLn("Error in PHYSFS_stat: {}", getPhysFSError());
		return nullptr;
	}

	if(info.filetype != PHYSFS_FILETYPE_REGULAR){
		logErrorLn("Tried to open directory as file");
		return nullptr;
	}

	auto file = access_ == Asset::Access::read
				? PHYSFS_openRead(pathStr.c_str())
				: PHYSFS_openWrite(pathStr.c_str());
	if(!file){
		logErrorLn("Error in PHYSFS_open: {}", getPhysFSError());
		return nullptr;
	}

	Vector<char> buf;

	buf.resize(info.filesize);

	if(PHYSFS_readBytes(file, buf.data(), buf.size()) != buf.size()){
		logErrorLn("Error in PHYSFS_open: {}", getPhysFSError());
		PHYSFS_close(file);
		return nullptr;
	}

	PHYSFS_close(file);

	std::uint8_t flags = (std::uint8_t)Asset::Access::read;

	if(!info.readonly){
		flags |= (std::uint8_t)Asset::Access::write;
	}

	StrView mimeFull = magic_buffer(gpweMagic, buf.data(), buf.size());
	StrView mimeType = mimeFull.substr(0, mimeFull.find_first_of(';'));
	StrView mimeCat = mimeType.substr(0, mimeType.find_first_of('/'));

	UniquePtr<Asset> asset;

	Asset::Category cat = Asset::Category::binary;

	logLn("Loading file '{}' with MIME type '{}'", path, mimeType);

	Str extension = fs::path(path).extension().string<char, std::char_traits<char>, Allocator<char>>();

	// TODO: load models
	if(mimeType == "text/plain"){
		if(extension == ".obj" || extension == ".OBJ"){
			cat = Asset::Category::model;
		}
	}
	else if(mimeType == "application/octet-stream"){
		if(extension == ".fbx" || extension == ".FBX"){
			cat = Asset::Category::model;
		}
	}
	else{
		if(mimeCat == "text"){

		}
		else if(mimeCat == "image"){

		}
		else if(mimeCat == "font"){

		}
		else if(mimeCat == "audio"){

		}
		else if(mimeCat == "video"){

		}
	}

	if(cat == Asset::Category::binary){
		asset = makeUnique<resource::BinaryFile>((Asset::Access)flags, Str(path), std::move(buf));
	}
	else if(cat == Asset::Category::model){
		asset = createModelFileAsset(Str(path), (Asset::Access)flags, std::move(buf));
	}

	auto ret = asset.get();

	if(!ret){
		logErrorLn("error opening asset");
		return nullptr;
	}

	m_assets.emplace_back(std::move(asset));

	m_files.emplace(path, ret);

	auto fileFlags = (std::uint8_t)ret->access();
	auto openFlags = (std::uint8_t)access_;
	if((fileFlags & openFlags) != openFlags){
		logErrorLn("Can not open file with access flags 0x{:h}", openFlags);
		return nullptr;
	}

	return ret;
}

resource::Model *resource::Manager::openModel(StrView path, Asset::Access access_){
	auto asset = openFile(path, access_);
	if(!asset){
		logErrorLn("Failed to open model file");
		return nullptr;
	}

	auto ret = dynamic_cast<resource::ModelFile*>(asset);
	if(!ret){
		logErrorLn("File is not a model: {}", path);
		return nullptr;
	}

	return ret;
}

void resource::Manager::update(){
	for(auto &&ptr : m_assets){
		ptr->update();
	}
}

UniquePtr<resource::Model> resource::Manager::createModelFileAsset(
	Str path, Asset::Access access_,
	Vector<char> bytes
){
	Assimp::Importer importer;

	auto scene = importer.ReadFileFromMemory(
		bytes.data(), bytes.size(),
		aiProcess_GenSmoothNormals | aiProcess_GenUVCoords | aiProcess_Triangulate |
		aiProcess_SortByPType | aiProcess_MakeLeftHanded | aiProcess_ImproveCacheLocality |
		aiProcess_OptimizeMeshes
	);

	if(!scene){
		logErrorLn("Could not import model: {}", importer.GetErrorString());
		return nullptr;
	}

	Vector<shapes::TriangleMesh> meshes;
	meshes.reserve(scene->mNumMeshes);

	for(std::uint32_t i = 0; i < scene->mNumMeshes; i++){
		auto mesh = scene->mMeshes[i];

		if(mesh->mNumUVComponents[0] == 0){
			logErrorLn("Could not import mesh '{}'", mesh->mName.C_Str());
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

		meshes.emplace_back(std::move(verts), std::move(norms), std::move(uvs), std::move(indices));
	}

	return makeUnique<resource::ModelFile>(access_, std::move(path), std::move(meshes));
}

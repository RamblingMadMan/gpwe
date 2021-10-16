#include "gpwe/log.hpp"
#include "gpwe/resource.hpp"

#include "ft2build.h"
#include FT_FREETYPE_H

#include "physfs.h"

#include "magic.h"

#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/Importer.hpp"

using GPWEProc = void(*)();

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
using LibHandle = HMODULE;
static LibHandle loadLibrary(const char *path){ return LoadLibraryA(path); }
static void closeLibrary(LibHandle lib){ FreeLibrary(lib); }
static const char *loadLibraryError(){
	static Str errMsg;
	errMsg = std::to_string(GetLastError());
	return errMsg.c_str();
}
static GPWEProc loadFunction(LibHandle lib, const char *name){
	return reinterpret_cast<GPWEProc>(GetProcAddress(lib, name));
}
#define LIB_PREFIX
#define LIB_EXT ".dll"
// LoadLibrary/FreeLibrary
#else
#include <dlfcn.h>
using LibHandle = void*;
static LibHandle loadLibrary(const char *path){ return dlopen(path, RTLD_LAZY); }
static void closeLibrary(LibHandle lib){ dlclose(lib); }
static const char *loadLibraryError(){ return dlerror(); }
static GPWEProc loadFunction(LibHandle lib, const char *name){
	return reinterpret_cast<GPWEProc>(dlsym(lib, name));
}
#define LIB_PREFIX "lib"
#define LIB_EXT ".so"
#endif

extern magic_t gpweMagic;
extern FT_Library gpweFtLib;

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

	using PluginStrFn = const char*(*)();
	using PluginVersionFn = Version(*)();
	using PluginKindFn = ManagerKind(*)();
	using PluginCreateFn = ManagerBase*(*)();

	class PluginFile: public Plugin{
		public:
			PluginFile(
				LibHandle handle_,
				PluginStrFn nameFn_,
				PluginStrFn authorFn_,
				PluginVersionFn versionFn_,
				PluginKindFn kindFn_,
				PluginCreateFn createFn_,
				Str path_
			)
				: Plugin(Kind::file, std::move(path_))
				, m_handle(handle_)
				, m_nameFn(nameFn_)
				, m_authorFn(authorFn_)
				, m_versionFn(versionFn_)
				, m_kindFn(kindFn_)
				, m_createFn(createFn_)
			{}

			~PluginFile(){
				closeLibrary(m_handle);
			}

			StrView name() const noexcept override{ return m_nameFn(); }
			StrView author() const noexcept override{ return m_authorFn(); }
			Version version() const noexcept override{ return m_versionFn(); }
			ManagerKind managerKind() const noexcept override{ return m_kindFn(); }

			UniquePtr<ManagerBase> createManager() const noexcept override{
				return UniquePtr{m_createFn()};
			}

			Proc loadFunction(const Str &name) const noexcept override{
				auto ret = ::loadFunction(m_handle, name.c_str());
				if(!ret){
					logErrorLn("Error in loadFunction: {}", loadLibraryError());
				}

				return ret;
			}

		private:
			LibHandle m_handle;
			PluginStrFn m_nameFn, m_authorFn;
			PluginVersionFn m_versionFn;
			PluginKindFn m_kindFn;
			PluginCreateFn m_createFn;
	};

	class ModelFile: public resource::Model{
		public:
			ModelFile(Str path_, Vector<shapes::TriangleMesh> meshes_)
				: Model(Kind::file, Access::read, std::move(path_), std::move(meshes_)){}
	};

	class FTFace: public Font::Face{
		public:
			FTFace(FT_Face face_)
				: m_face(face_){}

			~FTFace(){
				FT_Done_Face(m_face);
			}

			void setPtSize(std::uint16_t pt) override{
				FT_Set_Char_Size(m_face, 0, pt * 64, 96, 96);
			}

			StrView family() const noexcept override{
				return m_face->family_name;
			}

			StrView style() const noexcept override{
				return m_face->style_name;
			}

		private:
			FT_Face m_face;
	};

	class FontFile: public Font{
		public:
			FontFile(Str path_, Vector<char> bytes_, Vector<FT_Face> ftFaces)
				: Font(Kind::file, Access::read, std::move(path_))
				, m_bytes(std::move(bytes_))
			{
				m_faces.reserve(ftFaces.size());

				for(auto ftFace : ftFaces){
					m_faces.emplace_back(ftFace);
				}
			}

			~FontFile(){}

			std::size_t numFaces() const noexcept override{ return m_faces.size(); }

			FTFace *face(std::size_t idx) noexcept override{
				if(idx >= m_faces.size()) return nullptr;
				else return &m_faces[idx];
			}

			const FTFace *face(std::size_t idx) const noexcept override{
				if(idx >= m_faces.size()) return nullptr;
				else return &m_faces[idx];
			}

		private:
			Vector<char> m_bytes;
			Vector<FTFace> m_faces;
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

resource::Manager::~Manager(){}

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

resource::Asset *resource::Manager::openFile(StrView path, Access access_){
	if(access_ == Access::write){
		access_ = Access::readWrite;
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

	auto file = access_ == Access::read
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

	std::uint8_t flags = (std::uint8_t)Access::read;

	if(!info.readonly){
		flags |= (std::uint8_t)Access::write;
	}

	StrView mimeFull = magic_buffer(gpweMagic, buf.data(), buf.size());
	StrView mimeType = mimeFull.substr(0, mimeFull.find_first_of(';'));
	StrView mimeCat = mimeType.substr(0, mimeType.find_first_of('/'));

	UniquePtr<Asset> asset;

	Asset::Category cat = Asset::Category::binary;

	logLn("Loading file '{}' with MIME type '{}'", path, mimeType);

	Str filename = fs::path(path).filename().string<char, std::char_traits<char>, Allocator<char>>();

	Str extension = fs::path(path).extension().string<char, std::char_traits<char>, Allocator<char>>();

	// TODO: load models
	if(mimeType == "text/plain"){
		if(extension == ".obj" || extension == ".OBJ"){
			cat = Asset::Category::model;
		}
	}
	else if(mimeType == "application/x-sharedlib"){
		if(extension == LIB_EXT){
			if(filename.find(LIB_PREFIX "gpwe-") == 0){
				cat = Asset::Category::plugin;
			}
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
			cat = Asset::Category::font;
			flags = (std::uint8_t)Access::read;
		}
		else if(mimeCat == "audio"){

		}
		else if(mimeCat == "video"){

		}
	}

	if(cat == Asset::Category::plugin){
		asset = createPluginFileAsset(Str(path));
	}
	else if(cat == Asset::Category::model){
		asset = createModelFileAsset(Str(path), std::move(buf));
	}
	else if(cat == Asset::Category::font){
		asset = createFontFileAsset(Str(path), std::move(buf));
	}
	else{
		asset = makeUnique<resource::BinaryFile>((Access)flags, Str(path), std::move(buf));
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

resource::Plugin *resource::Manager::openPlugin(StrView path){
	auto asset = openFile(path, Access::read);
	if(!asset){
		logErrorLn("Failed to open plugin file '{}'", path);
		return nullptr;
	}

	auto ret = dynamic_cast<resource::Plugin*>(asset);
	if(!ret){
		logErrorLn("File is not a plugin: {}", path);
	}

	return ret;
}

resource::Font *resource::Manager::openFont(StrView path){
	auto asset = openFile(path, Access::read);
	if(!asset){
		logErrorLn("Failed to open font file '{}'", path);
		return nullptr;
	}

	auto ret = dynamic_cast<resource::Font*>(asset);
	if(!ret){
		logErrorLn("File is not a font: {}", path);
	}

	return ret;
}

resource::Model *resource::Manager::openModel(StrView path){
	auto asset = openFile(path, Access::read);
	if(!asset){
		logErrorLn("Failed to open model file '{}'", path);
		return nullptr;
	}

	auto ret = dynamic_cast<resource::ModelFile*>(asset);
	if(!ret){
		logErrorLn("File is not a model: {}", path);
	}

	return ret;
}

void resource::Manager::update(){
	for(auto &&ptr : m_assets){
		ptr->update();
	}
}

UniquePtr<resource::Plugin> resource::Manager::createPluginFileAsset(Str path){
	auto relPath = "./" + path;
	auto lib = loadLibrary(relPath.c_str());
	if(!lib){
		logErrorLn("Error in loadLibrary: {}", loadLibraryError());
		return nullptr;
	}

	auto nameFnPtr = loadFunction(lib, "gpwePluginName");
	auto authorFnPtr = loadFunction(lib, "gpwePluginAuthor");
	auto versionFnPtr = loadFunction(lib, "gpwePluginVersion");
	auto kindFnPtr = loadFunction(lib, "gpwePluginKind");
	auto createFnPtr = loadFunction(lib, "gpwePluginCreateManager");

	if(!nameFnPtr || !authorFnPtr || !versionFnPtr || !kindFnPtr || !createFnPtr){
		// not a plugin
		logLn("Skipping non-plugin library '{}'", path);
		return nullptr;
	}

	return makeUnique<resource::PluginFile>(
		lib,
		(resource::PluginStrFn)nameFnPtr,
		(resource::PluginStrFn)authorFnPtr,
		(resource::PluginVersionFn)versionFnPtr,
		(resource::PluginKindFn)kindFnPtr,
		(resource::PluginCreateFn)createFnPtr,
		std::move(path)
	);
}

UniquePtr<resource::Font> resource::Manager::createFontFileAsset(
	Str path,
	Vector<char> bytes
){
	FT_Open_Args args;
	args.memory_base = (FT_Byte*)bytes.data();
	args.memory_size = bytes.size();
	args.driver = nullptr;
	args.num_params = 0;
	args.params = nullptr;
	args.flags = FT_OPEN_MEMORY;
	args.stream = nullptr;

	FT_Face face;
	FT_Error err = FT_Open_Face(gpweFtLib, &args, -1, &face);
	if(err != FT_Err_Ok){
		logErrorLn("Error in FT_Open_Face('{}')", path);
		return nullptr;
	}

	auto numFaces = face->num_faces;

	FT_Done_Face(face);

	Vector<FT_Face> faces;
	faces.reserve(numFaces);

	for(FT_Long i = 0; i < numFaces; i++){
		err = FT_Open_Face(gpweFtLib, &args, i, &face);
		if(err != FT_Err_Ok){
			logErrorLn("Error opening face {} of font '{}'", i, path);
			continue;
		}

		faces.emplace_back(face);
	}

	return makeUnique<FontFile>(std::move(path), std::move(bytes), std::move(faces));
}

UniquePtr<resource::Model> resource::Manager::createModelFileAsset(
	Str path,
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

	return makeUnique<resource::ModelFile>(std::move(path), std::move(meshes));
}

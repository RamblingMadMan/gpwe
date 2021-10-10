#include "gpwe/log.hpp"
#include "gpwe/resource.hpp"

#include "physfs.h"

using namespace gpwe;

resource::Manager::Manager(){}

resource::Manager::~Manager(){
	for(auto &&p : m_memMap){
		if(p.first->kind() == Asset::Kind::file){
			PHYSFS_close(reinterpret_cast<PHYSFS_File*>(p.second));
		}
	}
}

bool resource::Manager::mount(const fs::path &path, std::string_view dir, bool mountBefore){
	if(!fs::exists(path)){
		return false;
	}

	auto pathStr = path.string();
	auto dirStr = std::string(dir);
	auto res = PHYSFS_mount(pathStr.c_str(), dirStr.c_str(), !mountBefore);
	if(!res){
		logErrorLn("{}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
	}

	return res != 0;
}

resource::Asset *resource::Manager::openFile(std::string_view path){
	auto pathStr = std::string(path);
	auto file = PHYSFS_openRead(pathStr.c_str());
	if(!file){
		logErrorLn("{}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
		return nullptr;
	}


}

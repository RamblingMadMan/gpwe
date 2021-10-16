#ifndef GPWE_APP_HPP
#define GPWE_APP_HPP 1

#include "Version.hpp"
#include "Manager.hpp"

namespace gpwe::app{
	class Manager: public gpwe::Manager<Manager>{
		public:
			virtual void update(float dt) = 0;
	};
}

#define GPWE_APP(type, name, author, major, minor, patch)\
extern "C" const char *gpweAppName(){ return name; }\
extern "C" const char *gpweAppAuthor(){ return author; }\
extern "C" gpwe::Version gpweAppVersion(){ return { major, minor, patch }; }\
extern "C" gpwe::app::Manager *gpweCreateAppManager(){\
	auto mem = gpwe::sys::alloc(sizeof(type));\
	return new(mem) type;\
}

#endif // !GPWE_APP_HPP

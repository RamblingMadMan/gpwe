#ifndef GPWE_APP_HPP
#define GPWE_APP_HPP 1

#include <memory>

#include "gpwe/Version.hpp"
#include "gpwe/String.hpp"

namespace gpwe{
	class App{
		public:
			virtual ~App() = default;
			virtual void update(float dt) = 0;
	};
}

#define GPWE_APP(type, name, author, major, minor, patch)\
extern "C" const char *gpweAppName(){ return name; }\
extern "C" const char *gpweAppAuthor(){ return author; }\
extern "C" gpwe::Version gpweAppVersion(){ return { major, minor, patch }; }\
extern "C" gpwe::App *gpweCreateApp(){\
	auto mem = gpwe::sys::alloc(sizeof(type));\
	return new(mem) type;\
}

#endif // !GPWE_APP_HPP

#ifndef GPWE_APP_HPP
#define GPWE_APP_HPP 1

#include <memory>

#include "gpwe/Version.hpp"
#include "gpwe/String.hpp"

namespace gpwe{
	class App{
		public:
			virtual ~App() = default;

			virtual StrView name() const noexcept = 0;
			virtual StrView author() const noexcept = 0;
			virtual Version version() const noexcept = 0;

			virtual void update(float dt) = 0;
	};
}

#define GPWE_APP(type, name, author, major, minor, patch)\
extern "C" const char *gpweAppName(){ return name; }\
extern "C" const char *gpweAppAuthor(){ return author; }\
extern "C" gpwe::Version gpweAppVersion(){ return { major, minor, patch }; }\
extern "C" gpwe::UniquePtr<gpwe::App> gpweCreateApp(){ return gpwe::makeUnique<type>(); }

#endif // !GPWE_APP_HPP

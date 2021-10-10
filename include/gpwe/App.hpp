#ifndef GPWE_APP_HPP
#define GPWE_APP_HPP 1

#include <memory>
#include <string_view>

#include "gpwe/Version.hpp"

namespace gpwe{
	class App{
		public:
			virtual ~App() = default;

			virtual std::string_view name() const noexcept = 0;
			virtual std::string_view author() const noexcept = 0;
			virtual Version version() const noexcept = 0;

			virtual void update(float dt) = 0;
	};
}

#define GPWE_APP(type, name, author, major, minor, patch)\
extern "C" const char *gpweAppName(){ return name; }\
extern "C" const char *gpweAppAuthor(){ return author; }\
extern "C" gpwe::Version gpweAppVersion(){ return { major, minor, patch }; }\
extern "C" std::unique_ptr<gpwe::App> gpweCreateApp(){ return std::make_unique<type>(); }

#endif // !GPWE_APP_HPP

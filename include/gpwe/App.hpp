#ifndef GPWE_APP_HPP
#define GPWE_APP_HPP 1

#include <memory>
#include <string_view>

namespace gpwe{
	struct Version{
		std::uint16_t major, minor, patch;
	};

	class App{
		public:
			virtual ~App() = default;

			virtual std::string_view name() const noexcept = 0;
			virtual std::string_view author() const noexcept = 0;
			virtual Version version() const noexcept = 0;

			virtual void update(float dt) = 0;
	};
}

#endif // !GPWE_APP_HPP

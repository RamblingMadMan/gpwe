#ifndef GPWE_APP_HPP
#define GPWE_APP_HPP 1

#include "Version.hpp"
#include "Manager.hpp"

namespace gpwe::app{
	class Manager:
		public Object<Manager>,
		public gpwe::Manager<Manager, ManagerKind::app>
	{
		public:
			virtual void update(float dt) = 0;
	};
}

#define GPWE_APP_PLUGIN(type, name, author, major, minor, patch)\
	GPWE_PLUGIN(app, type, name, author, major, minor, patch)

#endif // !GPWE_APP_HPP

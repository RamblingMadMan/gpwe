set(
	GPWE_PHYSICS_BULLET3_SOURCES
	physics-bullet3.hpp
	physics-bullet3.cpp
)

add_gpwe_plugin(physics-bullet3 ${GPWE_PHYSICS_BULLET3_SOURCES})

plugin_link_libraries(physics-bullet3 Bullet::Bullet3)

find_package(SDL2 REQUIRED)
find_package(Freetype REQUIRED)
find_package(FreeImage REQUIRED)

set(
	GPWE_CLIENT_SOURCES
	main.cpp
)

add_executable(gpwe-client ${GPWE_INCLUDES} ${GPWE_CLIENT_SOURCES})

set_target_properties(
	gpwe-client PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
)

target_include_directories(gpwe-client PRIVATE ${SDL2_INCLUDE_DIRS})

target_link_libraries(gpwe-client gpwe-base Freetype::Freetype FreeImage::FreeImage ${SDL2_LIBRARIES})

add_dependencies(gpwe-client gpwe-renderer-gl43)

find_package(SDL2 REQUIRED)

set(
	GPWE_CLIENT_SOURCES
	main.cpp
)

add_executable(gpwe-client ${GPWE_INCLUDES} ${GPWE_CLIENT_SOURCES})

set_target_properties(
	gpwe-client PROPERTIES
	ENABLE_EXPORTS ON
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
)

target_include_directories(gpwe-client PRIVATE ${SDL2_INCLUDE_DIRS})

target_link_libraries(gpwe-client gpwe-base ${SDL2_LIBRARIES})

gpwe_embed_renderer(gpwe-client gpwe-renderer-gl43)
gpwe_embed_app(gpwe-client gpwe-app-test)

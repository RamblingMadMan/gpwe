find_package(SDL2 REQUIRED)

set(
	GPWE_CLIENT_SOURCES
	main.cpp
)

add_executable(gpwe-client ${GPWE_INCLUDES} ${GPWE_CLIENT_SOURCES})

set_target_properties(
	gpwe-client PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
)

target_include_directories(gpwe-client PRIVATE ${SDL2_INCLUDE_DIRS})

target_link_libraries(gpwe-client PRIVATE ${SDL2_LIBRARIES} GPWE::Base)

add_custom_target(
	gpwe-client-config

	COMMAND
		${CMAKE_COMMAND} -E copy
			$<TARGET_PROPERTY:gpwe-client,SOURCE_DIR>/gpwe-config.toml
			$<TARGET_PROPERTY:gpwe-client,BINARY_DIR>/gpwe-config.toml

	DEPENDS
		gpwe-config.toml

	VERBATIM
)

add_dependencies(gpwe-client gpwe-client-config)

gpwe_embed_plugins(gpwe-client renderer-gl43 physics-bullet3 world-simple)

if(GPWE_BUILD_TESTGAME)
	gpwe_embed_app(gpwe-client app-test)
endif()

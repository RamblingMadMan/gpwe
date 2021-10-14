find_package(LibMagic REQUIRED)
find_package(Freetype REQUIRED)
find_package(FreeImage REQUIRED)
find_package(Bullet REQUIRED)

set(
	GPWE_BASE_SOURCES
	sys.cpp
	input.cpp
	resource.cpp
	Camera.cpp
	Shape.cpp
	Renderer.cpp
	Font.cpp
	World.cpp
)

add_library(gpwe-base SHARED ${GPWE_INCLUDES} ${GPWE_BASE_SOURCES})

set_target_properties(
	gpwe-base PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
)

target_include_directories(gpwe-base PUBLIC ${GPWE_INCLUDE_DIRS})

target_include_directories(
	gpwe-base PRIVATE
	${PHYSFS_INCLUDE_DIRS}
	${LIBMAGIC_INCLUDE_DIRS}
)

target_link_libraries(
	gpwe-base PRIVATE
	${CMAKE_DL_LIBS}
	fmt
	glm::glm_static
	physfs-static
	${LIBMAGIC_LIBRARIES}
	Freetype::Freetype
	FreeImage::FreeImage
	assimp
)

target_link_libraries(
	gpwe-base PUBLIC
	glm::glm
	fmt-header-only
	plf-list
)

# Usage: add_gpwe_renderer(my-renderer MyRenderer.hpp MyRenderer.cpp)
function(add_gpwe_renderer name)
	add_library(${name} SHARED ${ARGN})
	target_link_libraries(${name} PUBLIC gpwe-base)
	set_target_properties(
		${name} PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED ON
	)
endfunction()

# Usage: add_gpwe_app(my-app MyApp.hpp MyApp.cpp)
function(add_gpwe_app name)
	add_library(${name} SHARED ${ARGN})
	target_link_libraries(${name} PUBLIC gpwe-base)
	set_target_properties(
		${name} PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED ON
	)
endfunction()

function(gpwe_embed_renderer client renderer)
	target_link_libraries(${client} PRIVATE gpwe-base)

	add_dependencies(${client} ${renderer})

	add_custom_command(TARGET ${client} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${renderer}> $<TARGET_FILE_DIR:${client}>
	)
endfunction()

function(gpwe_embed_app client app)
	target_link_libraries(${client} PRIVATE gpwe-base)

	add_dependencies(${client} ${app})

	add_custom_command(TARGET ${client} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${app}> $<TARGET_FILE_DIR:${client}>
	)

	add_custom_command(
		TARGET ${client} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory $<TARGET_PROPERTY:${app},SOURCE_DIR>/Assets $<TARGET_FILE_DIR:${client}>/Assets
	)
endfunction()

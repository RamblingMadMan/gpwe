find_package(LibMagic REQUIRED)
find_package(Freetype REQUIRED)
find_package(FreeImage REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

set(GPWE_BASE_DIR ${CMAKE_CURRENT_LIST_DIR} CACHE INTERNAL "GPWE base directory")

set(
	GPWE_BASE_SOURCES
	meta.cpp
	Object.cpp
	Thread.cpp
	sys.cpp
	input.cpp
	resource.cpp
	physics.cpp
	Camera.cpp
	Shape.cpp
	World.cpp
	ui.cpp
)

add_library(gpwe-base SHARED ${GPWE_INCLUDES} ${GPWE_BASE_SOURCES})

set_target_properties(
	gpwe-base PROPERTIES
	CXX_STANDARD 20
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
	Threads::Threads
	fmt
	FastNoise::FastNoise2
	glm::glm_static
	physfs-static
	${LIBMAGIC_LIBRARIES}
	Freetype::Freetype
	FreeImage::FreeImage
	assimp
)

target_link_libraries(
	gpwe-base PUBLIC
	robin_hood::robin_hood
	glm::glm
	fmt-header-only
	plf-list
)

add_library(GPWE::Base ALIAS gpwe-base)

# Usage: add_gpwe_plugin(plugin plugin.hpp plugin.cpp)
macro(add_gpwe_plugin name)
	if(${ARGC} LESS 2)
		message(FATAL_ERROR "add_gpwe_plugin(${name}) called with no sources")
	endif()

	add_library(gpwe-${name} SHARED ${ARGN})
	target_link_libraries(gpwe-${name} PUBLIC GPWE::Base)
	target_include_directories(gpwe-${name} PRIVATE $<TARGET_PROPERTY:gpwe-${name},BINARY_DIR>)
	set_target_properties(
		gpwe-${name} PROPERTIES
		POSITION_INDEPENDENT_CODE ON
		LANGUAGE CXX
		CXX_STANDARD 20
		CXX_STANDARD_REQUIRED ON
	)
endmacro()

# Usage: plugin_add_resources(plugin mydata.json)
# then in plugin.cpp: #include "mydata.json.hpp"
function(plugin_add_resources plugin)
	if(${ARGC} LESS 2)
		message(WARNING "plugin_add_resources(${plugin}) called with no resources")
		return()
	endif()

	if(NOT TARGET gpwe-${plugin}-res)
		add_custom_target(
			gpwe-${plugin}-res

			WORKING_DIRECTORY
				$<TARGET_PROPERTY:gpwe-${plugin},BINARY_DIR>

			SOURCES
				${ARGN}
				$<TARGET_PROPERTY:gpwe-${plugin}-res,RESOURCES>
		)

		add_dependencies(gpwe-${plugin} gpwe-${plugin}-res)
	else()
		set_property(TARGET gpwe-${plugin}-res APPEND PROPERTY RESOURCES ${ARGN})
	endif()

	foreach(res IN ITEMS ${ARGN})
		string(REGEX REPLACE "\\.|/" "_" RES_NAME "${res}")
		set(RES_SRC "${res}")
		set(RES_OBJ "$<TARGET_PROPERTY:gpwe-${plugin},BINARY_DIR>/${res}.o")
		set(RES_HEADER "$<TARGET_PROPERTY:gpwe-${plugin},BINARY_DIR>/${res}.hpp")

		message(STATUS "Creating header '${res}.hpp' for resource '${RES_NAME}'")

		configure_file(${GPWE_BASE_DIR}/embed.hpp.in ${res}.hpp)

		message(STATUS "Creating object '${res}.o' for resource '${RES_NAME}'")

		add_custom_target(
			gpwe-${plugin}-res-${RES_NAME}

			COMMAND
				${CMAKE_COMMAND} -E copy $<TARGET_PROPERTY:gpwe-${plugin},SOURCE_DIR>/${res} ${res}

			COMMAND
				${TOOLCHAIN_PREFIX}ld -r -b binary
				-o "${RES_OBJ}"
				"${res}"

			COMMAND
				${CMAKE_COMMAND} -E remove ${res}

			WORKING_DIRECTORY
				$<TARGET_PROPERTY:gpwe-${plugin},BINARY_DIR>

			DEPENDS
				$<TARGET_PROPERTY:gpwe-${plugin},SOURCE_DIR>/${res}

			VERBATIM
		)

		add_dependencies(gpwe-${plugin}-res gpwe-${plugin}-res-${RES_NAME})

		target_link_libraries(gpwe-${plugin} PRIVATE ${RES_OBJ})
		target_sources(gpwe-${plugin} PRIVATE ${RES_HEADER})
	endforeach()
endfunction()

# Usage: plugin_link_libraries plugin lib1 lib2
macro(plugin_link_libraries plugin)
	if(${ARGC} LESS 2)
		message(WARNING "plugin_link_libraries(${name}) called with no libraries")
		return()
	endif()
	target_link_libraries(gpwe-${plugin} PRIVATE ${ARGN})
endmacro()

macro(plugin_sources plugin)
	if(${ARGC} LESS 2)
		message(WARNING "plugin_link_libraries(${name}) called with no libraries")
	endif()
endmacro()

# Usage: add_gpwe_app(my-app MyApp.hpp MyApp.cpp)
function(add_gpwe_app name)
	if(${ARGC} LESS 2)
		message(WARNING "add_gpwe_app(${name}) called with no sources")
		return()
	endif()
	add_gpwe_plugin(${name} ${ARGN})
endfunction()

# Usage: gpwe_embed_plugins(my-game cool-app great-renderer)
function(gpwe_embed_plugins tgt)
	if(${ARGC} LESS 2)
		message(WARNING "gpwe_embed_plugins(${tgt}) has no plugins specified")
		return()
	endif()

	foreach(plugin IN ITEMS ${ARGN})
		add_dependencies(${tgt} gpwe-${plugin})

		add_custom_command(
			OUTPUT ${CMAKE_SHARED_LIBRARY_PREFIX}gpwe-${plugin}${CMAKE_SHARED_LIBRARY_SUFFIX}

			COMMAND
				${CMAKE_COMMAND} -E copy $<TARGET_FILE:gpwe-${plugin}> $<TARGET_PROPERTY:${tgt},BINARY_DIR>

			DEPENDS
				gpwe-${plugin}

			WORKING_DIRECTORY
				$<TARGET_PROPERTY:${tgt},BINARY_DIR>

			VERBATIM
		)

		add_custom_target(
			gpwe-${tgt}-${plugin}

			WORKING_DIRECTORY
				$<TARGET_PROPERTY:${tgt},BINARY_DIR>

			DEPENDS
				${CMAKE_SHARED_LIBRARY_PREFIX}gpwe-${plugin}${CMAKE_SHARED_LIBRARY_SUFFIX}
		)

		add_dependencies(${tgt} gpwe-${tgt}-${plugin})
	endforeach()
endfunction()

function(gpwe_embed_app client app)
	gpwe_embed_plugins(${client} ${app})

	add_custom_command(
		TARGET ${client} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory $<TARGET_PROPERTY:gpwe-${app},SOURCE_DIR>/Assets $<TARGET_FILE_DIR:${client}>/Assets
	)
endfunction()

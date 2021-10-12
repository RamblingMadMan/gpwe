cmake_minimum_required(VERSION 3.14.0 FATAL_ERROR)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

project(gpwe VERSION 0.0.0 LANGUAGES C CXX)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	set(GPWE_MASTER_PROJECT TRUE)
else()
	set(GPWE_MASTER_PROJECT FALSE)
endif()

option(GPWE_BUILD_EDITOR "Build the GPWE editor" ${GPWE_MASTER_PROJECT})
option(GPWE_BUILD_CLIENT "Build the GPWE client" ${GPWE_MASTER_PROJECT})
option(GPWE_BUILD_TESTGAME "Build the GPWE test game" ${GPWE_MASTER_PROJECT})

execute_process(
	COMMAND git rev-parse --short HEAD
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
	OUTPUT_VARIABLE GPWE_GIT_HASH
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(GPWE_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(GPWE_INCLUDE_DIRS ${GPWE_INCLUDE_DIR} ${PROJECT_BINARY_DIR}/include)

set(
	GPWE_INCLUDES
	${PROJECT_BINARY_DIR}/include/gpwe/config.hpp
	${GPWE_INCLUDE_DIR}/gpwe/log.hpp
	${GPWE_INCLUDE_DIR}/gpwe/Version.hpp
	${GPWE_INCLUDE_DIR}/gpwe/sys.hpp
	${GPWE_INCLUDE_DIR}/gpwe/Allocator.hpp
	${GPWE_INCLUDE_DIR}/gpwe/Ticker.hpp
	${GPWE_INCLUDE_DIR}/gpwe/Vector.hpp
	${GPWE_INCLUDE_DIR}/gpwe/List.hpp
	${GPWE_INCLUDE_DIR}/gpwe/Map.hpp
	${GPWE_INCLUDE_DIR}/gpwe/String.hpp
	${GPWE_INCLUDE_DIR}/gpwe/input.hpp
	${GPWE_INCLUDE_DIR}/gpwe/resource.hpp
	${GPWE_INCLUDE_DIR}/gpwe/Octree.hpp
	${GPWE_INCLUDE_DIR}/gpwe/Camera.hpp
	${GPWE_INCLUDE_DIR}/gpwe/Shape.hpp
	${GPWE_INCLUDE_DIR}/gpwe/Texture.hpp
	${GPWE_INCLUDE_DIR}/gpwe/Renderer.hpp
	${GPWE_INCLUDE_DIR}/gpwe/App.hpp
	${GPWE_INCLUDE_DIR}/gpwe/World.hpp
	${GPWE_INCLUDE_DIR}/gpwe/Font.hpp
)

configure_file(${GPWE_INCLUDE_DIR}/gpwe/config.hpp.in include/gpwe/config.hpp)

# Dependencies

add_library(plf-list INTERFACE)

target_include_directories(plf-list INTERFACE deps/plf_list)

set(PHYSFS_BUILD_TEST OFF CACHE BOOL "")
set(PHYSFS_BUILD_DOCS OFF CACHE BOOL "")
add_subdirectory(deps/physfs EXCLUDE_FROM_ALL)
set_property(TARGET physfs-static PROPERTY POSITION_INDEPENDENT_CODE ON)

set(PHYSFS_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/deps/physfs/src)

set(BUILD_STATIC_LIBS ON CACHE BOOL "")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "")

add_subdirectory(deps/glm EXCLUDE_FROM_ALL)
add_subdirectory(deps/fmt EXCLUDE_FROM_ALL)

set(OPTION_BUILD_TOOLS OFF CACHE BOOL "")
set(OPTION_BUILD_EXAMPLES OFF CACHE BOOL "")
add_subdirectory(deps/glbinding EXCLUDE_FROM_ALL)

set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "")
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "")
set(ASSIMP_INJECT_DEBUG_POSTFIX OFF CACHE BOOL "")
set(ASSIMP_INSTALL OFF CACHE BOOL "")
add_subdirectory(deps/assimp EXCLUDE_FROM_ALL)

# GPWE targets

add_subdirectory(base)
add_subdirectory(renderer-gl43)

if(GPWE_BUILD_CLIENT)
	add_subdirectory(client)
endif()

if(GPWE_BUILD_TESTGAME)
	add_subdirectory(testgame)
endif()

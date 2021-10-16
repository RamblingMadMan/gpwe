cmake_minimum_required(VERSION 3.14.0 FATAL_ERROR)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

project(gpwe VERSION 0.0.1 LANGUAGES C CXX)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	set(GPWE_MASTER_PROJECT TRUE)
else()
	set(GPWE_MASTER_PROJECT FALSE)
endif()

option(GPWE_BUILD_EDITOR "Build the GPWE editor" ${GPWE_MASTER_PROJECT})
option(GPWE_BUILD_CLIENT "Build the GPWE client" ${GPWE_MASTER_PROJECT})
option(GPWE_BUILD_DOCS "Build the GPWE docs" ${GPWE_MASTER_PROJECT})
option(GPWE_BUILD_TESTGAME "Build the GPWE test game" ${GPWE_MASTER_PROJECT})
option(GPWE_BUILD_TESTEMBED "Build the GPWE embedding test app" ${GPWE_MASTER_PROJECT})

execute_process(
	COMMAND git rev-parse --short HEAD
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
	OUTPUT_VARIABLE GPWE_GIT_HASH
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
	COMMAND git rev-parse --abbrev-ref HEAD
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
	OUTPUT_VARIABLE GPWE_GIT_BRANCH
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(GPWE_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(GPWE_INCLUDE_DIRS ${GPWE_INCLUDE_DIR} ${PROJECT_BINARY_DIR}/include)

set(
	GPWE_INCLUDES
	${PROJECT_BINARY_DIR}/include/gpwe/config.hpp
	${GPWE_INCLUDE_DIR}/gpwe/meta.hpp
	${GPWE_INCLUDE_DIR}/gpwe/log.hpp
	${GPWE_INCLUDE_DIR}/gpwe/Version.hpp
	${GPWE_INCLUDE_DIR}/gpwe/Allocator.hpp
	${GPWE_INCLUDE_DIR}/gpwe/Manager.hpp
	${GPWE_INCLUDE_DIR}/gpwe/WorkQueue.hpp
	${GPWE_INCLUDE_DIR}/gpwe/sys.hpp
	${GPWE_INCLUDE_DIR}/gpwe/render.hpp
	${GPWE_INCLUDE_DIR}/gpwe/physics.hpp
	${GPWE_INCLUDE_DIR}/gpwe/app.hpp
	${GPWE_INCLUDE_DIR}/gpwe/embed.hpp
	${GPWE_INCLUDE_DIR}/gpwe/Ticker.hpp
	${GPWE_INCLUDE_DIR}/gpwe/Vector.hpp
	${GPWE_INCLUDE_DIR}/gpwe/List.hpp
	${GPWE_INCLUDE_DIR}/gpwe/Map.hpp
	${GPWE_INCLUDE_DIR}/gpwe/Str.hpp
	${GPWE_INCLUDE_DIR}/gpwe/input.hpp
	${GPWE_INCLUDE_DIR}/gpwe/resource.hpp
	${GPWE_INCLUDE_DIR}/gpwe/Octree.hpp
	${GPWE_INCLUDE_DIR}/gpwe/Camera.hpp
	${GPWE_INCLUDE_DIR}/gpwe/Shape.hpp
	${GPWE_INCLUDE_DIR}/gpwe/Texture.hpp
	${GPWE_INCLUDE_DIR}/gpwe/World.hpp
	${GPWE_INCLUDE_DIR}/gpwe/Font.hpp
)

configure_file(${GPWE_INCLUDE_DIR}/gpwe/config.hpp.in include/gpwe/config.hpp)

# Dependencies

set(CMAKE_POLICY_DEFAULT_CMP0072 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)

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

set(USE_GRAPHICAL_BENCHMARK OFF CACHE BOOL "")
set(BULLET2_MULTITHREADING ON CACHE BOOL "")
set(BULLET2_USE_OPEN_MP_MULTITHREADING ON CACHE BOOL "")
add_subdirectory(deps/bullet3 EXCLUDE_FROM_ALL)

set_target_properties(
	Bullet3Collision PROPERTIES
	POSITION_INDEPENDENT_CODE ON
)

set_target_properties(
	Bullet3Dynamics PROPERTIES
	POSITION_INDEPENDENT_CODE ON
)

add_library(Bullet::LinearMath INTERFACE IMPORTED)

target_link_libraries(Bullet::LinearMath INTERFACE LinearMath)

target_include_directories(
	Bullet::LinearMath INTERFACE
	deps/bullet3/src
)

add_library(Bullet::Bullet3 INTERFACE IMPORTED)

target_link_libraries(
	Bullet::Bullet3 INTERFACE
	Bullet::LinearMath
	Bullet3Common
	Bullet3Collision
	Bullet3Dynamics
	Bullet3Geometry
	#Bullet3OpenCL
)

# GPWE targets

add_subdirectory(base)
add_subdirectory(renderer-gl43)
add_subdirectory(physics-bullet3)

if(GPWE_BUILD_DOCS)
	add_subdirectory(docs)
endif()

if(GPWE_BUILD_TESTGAME)
	add_subdirectory(testgame)
endif()

if(GPWE_BUILD_CLIENT)
	add_subdirectory(sys-sdl2)
	add_subdirectory(client)
endif()

if(GPWE_BUILD_TESTEMBED)
	add_subdirectory(testembed)
endif()

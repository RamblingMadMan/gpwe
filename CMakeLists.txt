cmake_minimum_required(VERSION 3.14.0 FATAL_ERROR)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

project(gpwe VERSION 0.0.0 LANGUAGES C CXX)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	set(GPWE_MASTER_PROJECT TRUE)
else()
	set(GPWE_MASTER_PROJECT FALSE)
endif()

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
	${GPWE_INCLUDE_DIR}/gpwe/sys.hpp
	${GPWE_INCLUDE_DIR}/gpwe/Camera.hpp
	${GPWE_INCLUDE_DIR}/gpwe/Shape.hpp
	${GPWE_INCLUDE_DIR}/gpwe/Texture.hpp
	${GPWE_INCLUDE_DIR}/gpwe/Model.hpp
	${GPWE_INCLUDE_DIR}/gpwe/Renderer.hpp
	${GPWE_INCLUDE_DIR}/gpwe/App.hpp
)

configure_file(${GPWE_INCLUDE_DIR}/gpwe/config.hpp.in include/gpwe/config.hpp)

# Dependencies

add_library(plf-list INTERFACE)

target_include_directories(plf-list INTERFACE deps/plf_list)

add_subdirectory(deps/glm EXCLUDE_FROM_ALL)
add_subdirectory(deps/fmt EXCLUDE_FROM_ALL)
add_subdirectory(deps/glbinding EXCLUDE_FROM_ALL)

function(addAssimp)
	set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
	set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "")
	set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "")
	set(ASSIMP_INJECT_DEBUG_POSTFIX OFF CACHE BOOL "")
	set(ASSIMP_INSTALL OFF CACHE BOOL "")
	add_subdirectory(deps/assimp EXCLUDE_FROM_ALL)
endfunction()

addAssimp()

# GPWE targets

add_subdirectory(base)
add_subdirectory(renderer-gl43)

if(GPWE_MASTER_PROJECT)
	add_subdirectory(testgame)
	add_subdirectory(client)
endif()

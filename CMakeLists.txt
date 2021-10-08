cmake_minimum_required(VERSION 3.14.0 FATAL_ERROR)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

project(gpwe VERSION 0.0.0 LANGUAGES CXX)

execute_process(
	COMMAND git log -1 --format=%h
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
)

configure_file(${GPWE_INCLUDE_DIR}/gpwe/config.hpp.in include/gpwe/config.hpp)

# Dependencies

add_subdirectory(deps/fmt)

# GPWE targets

add_subdirectory(client)

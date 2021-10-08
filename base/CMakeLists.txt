set(
	GPWE_BASE_SOURCES
	Shape.cpp
)

add_library(gpwe-base SHARED ${GPWE_INCLUDES} ${GPWE_BASE_SOURCES})

target_include_directories(gpwe-base PUBLIC ${GPWE_INCLUDE_DIRS})

target_link_libraries(gpwe-base PUBLIC glm fmt plf-list)

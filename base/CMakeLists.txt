set(
	GPWE_BASE_SOURCES
	Camera.cpp
	Shape.cpp
	Model.cpp
	Renderer.cpp
)

add_library(gpwe-base SHARED ${GPWE_INCLUDES} ${GPWE_BASE_SOURCES})

set_target_properties(
	gpwe-base PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
)

target_include_directories(gpwe-base PUBLIC ${GPWE_INCLUDE_DIRS})

target_link_libraries(gpwe-base PRIVATE fmt)

target_link_libraries(gpwe-base PUBLIC glm fmt-header-only plf-list assimp)

set(
	GPWE_RENDERER_GL43_SOURCES
	RendererGL43.hpp
	RendererGL43.cpp
)

add_library(gpwe-renderer-gl43 SHARED ${GPWE_RENDERER_GL43_SOURCES})

set_target_properties(
	gpwe-renderer-gl43 PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
)

target_link_libraries(gpwe-renderer-gl43 PRIVATE glbinding)
target_link_libraries(gpwe-renderer-gl43 PUBLIC gpwe-base)

add_custom_command(TARGET gpwe-renderer-gl43 POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:gpwe-renderer-gl43> ${CMAKE_BINARY_DIR}/client/
)


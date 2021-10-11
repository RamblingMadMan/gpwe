set(
	GPWE_RENDERER_GL43_SOURCES
	shaders.hpp
	RendererGL43.hpp
	RenderProgramGL43.cpp
	RenderPipelineGL43.cpp
	RenderFramebufferGL43.cpp
	RendererGL43.cpp
)

add_library(gpwe-renderer-gl43 SHARED ${GPWE_RENDERER_GL43_SOURCES})

set_target_properties(
	gpwe-renderer-gl43 PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
)

target_link_libraries(gpwe-renderer-gl43 PRIVATE glbinding glbinding-aux)
target_link_libraries(gpwe-renderer-gl43 PUBLIC gpwe-base)

if(GPWE_BUILD_CLIENT)
	add_custom_command(TARGET gpwe-renderer-gl43 POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:gpwe-renderer-gl43> ${CMAKE_BINARY_DIR}/client/
	)
endif()

set(
	GPWE_RENDERER_GL43_SOURCES
	RendererGL43.hpp
	RendererGL43.cpp
	RenderProgramGL43.cpp
	RenderPipelineGL43.cpp
	RenderFramebufferGL43.cpp
)

set(
	GPWE_RENDERER_GL43_RESOURCES
	shaders/fullbright.vert
	shaders/fullbright.frag
)

add_gpwe_plugin(renderer-gl43 ${GPWE_RENDERER_GL43_SOURCES})

plugin_add_resources(renderer-gl43 ${GPWE_RENDERER_GL43_RESOURCES})

plugin_link_libraries(renderer-gl43 glbinding glbinding-aux)

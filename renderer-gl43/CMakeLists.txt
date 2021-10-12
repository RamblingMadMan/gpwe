set(
	GPWE_RENDERER_GL43_SOURCES
	shaders.hpp
	RendererGL43.hpp
	RenderProgramGL43.cpp
	RenderPipelineGL43.cpp
	RenderFramebufferGL43.cpp
	RendererGL43.cpp
)

add_gpwe_renderer(gpwe-renderer-gl43 ${GPWE_RENDERER_GL43_SOURCES})

target_link_libraries(gpwe-renderer-gl43 PRIVATE glbinding glbinding-aux)

function(gpwe_embed_renderer_gl43 tgt)
	gpwe_embed_renderer(${tgt} gpwe-renderer-gl43)
endfunction()

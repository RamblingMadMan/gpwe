#include "glbinding/gl43core/gl.h"

#include "RendererGL43.hpp"

using namespace gl;
using namespace gpwe;

namespace {
	UseProgramStageMask shaderKindToGLBit(render::Program::Kind kind){
		using Kind = render::Program::Kind;
		switch(kind){
			case Kind::vertex: return GL_VERTEX_SHADER_BIT;
			case Kind::geometry: return GL_GEOMETRY_SHADER_BIT;
			case Kind::fragment: return GL_FRAGMENT_SHADER_BIT;
			case Kind::compute: return GL_COMPUTE_SHADER_BIT;
			default: return GL_FRAGMENT_SHADER_BIT;
		}
	}
}

RenderPipelineGL43::RenderPipelineGL43(const Vector<RenderProgramGL43*> &progs)
	: m_progs(progs)
{
	glCreateProgramPipelines(1, &m_handle);

	for(auto prog : progs){
		glUseProgramStages(m_handle, shaderKindToGLBit(prog->kind()), prog->handle());
	}

	glValidateProgramPipeline(m_handle);
}

RenderPipelineGL43::~RenderPipelineGL43(){
	glDeleteProgramPipelines(1, &m_handle);
}

void RenderPipelineGL43::use() const noexcept{
	glBindProgramPipeline(m_handle);
}

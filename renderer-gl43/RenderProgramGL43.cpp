#include "glbinding/gl43core/gl.h"

#include "RendererGL43.hpp"

using namespace gpwe;
using namespace gl;

namespace {
	GLenum shaderKindToGL(render::Program::Kind kind){
		using Kind = render::Program::Kind;
		switch(kind){
			case Kind::vertex: return GL_VERTEX_SHADER;
			case Kind::geometry: return GL_GEOMETRY_SHADER;
			case Kind::fragment: return GL_FRAGMENT_SHADER;
			case Kind::compute: return GL_COMPUTE_SHADER;
			default: return GL_FRAGMENT_SHADER;
		}
	}
}

RenderProgramGL43::RenderProgramGL43(Kind kind_, std::string_view src)
	: m_kind(kind_), m_handle(glCreateProgram())
{
	auto shad = glCreateShader(shaderKindToGL(kind_));

	const char *strs[] = { src.data() };
	GLint lengths[] = { (GLint)src.size() };

	glShaderSource(shad, 1, strs, lengths);

	glCompileShader(shad);

	GLint res;
	glGetShaderiv(shad, GL_COMPILE_STATUS, &res);

	if(res != (GLint)GL_TRUE){
		glGetShaderiv(shad, GL_INFO_LOG_LENGTH, &res);
		std::string msg(res, '\0');
		glGetShaderInfoLog(shad, msg.size(), &res, msg.data());
		throw std::runtime_error(msg);
	}

	glProgramParameteri(m_handle, GL_PROGRAM_SEPARABLE, GL_TRUE);

	glAttachShader(m_handle, shad);

	glLinkProgram(m_handle);

	glDeleteShader(shad);

	glGetProgramiv(m_handle, GL_LINK_STATUS, &res);

	if(res != (GLint)GL_TRUE){
		glGetProgramiv(m_handle, GL_INFO_LOG_LENGTH, &res);
		std::string msg(res, '\0');
		glGetProgramInfoLog(m_handle, msg.size(), &res, msg.data());
		throw std::runtime_error(msg);
	}

	glValidateProgram(m_handle);
}

RenderProgramGL43::~RenderProgramGL43(){
	glDeleteProgram(m_handle);
}

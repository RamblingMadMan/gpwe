#include "glbinding/gl43core/gl.h"

#include "RendererGL43.hpp"

using namespace gl;
using namespace gpwe;

namespace {
	constexpr GLenum fbModeToGL(RenderFramebuffer::Mode mode) noexcept{
		using Mode = RenderFramebuffer::Mode;
		switch(mode){
			case Mode::read: return GL_READ_FRAMEBUFFER;
			case Mode::write: return GL_DRAW_FRAMEBUFFER;
			default: return GL_FRAMEBUFFER;
		}
	}

	constexpr GLenum textureKindToGLFormat(Texture::Kind kind) noexcept{
		using Kind = Texture::Kind;
		switch(kind){
#define CASE_RGBA(n)\
			case Kind::r##n: return GL_R##n;\
			case Kind::rg##n: return GL_RG##n;\
			case Kind::rgb##n: return GL_RGB##n;\
			case Kind::rgba##n: return GL_RGBA##n

#define CASE_RGBA_SUF(n, suf, sufGL)\
			case Kind::r##n##suf: return GL_R##n##sufGL;\
			case Kind::rg##n##suf: return GL_RG##n##sufGL;\
			case Kind::rgb##n##suf: return GL_RGB##n##sufGL;\
			case Kind::rgba##n##suf: return GL_RGBA##n##sufGL

#define CASE_RGBA_TYPED(bits)\
			CASE_RGBA_SUF(bits, n, UI);\
			CASE_RGBA_SUF(bits, i, I);\
			CASE_RGBA_SUF(bits, f, F)

			CASE_RGBA(8);
			CASE_RGBA(16);

			CASE_RGBA_TYPED(16);
			CASE_RGBA_TYPED(32);

			case Kind::rgb10a2: return GL_RGB10_A2;

			case Kind::d16: return GL_DEPTH_COMPONENT16;
			case Kind::d32: return GL_DEPTH_COMPONENT32;
			case Kind::d32f: return GL_DEPTH_COMPONENT32F;
			case Kind::d24s8: return GL_DEPTH24_STENCIL8;

#undef CASE_RGBA
#undef CASE_RGBA_SUF
#undef CASE_RGBA_TYPED

			default: return GL_RGBA8;
		}
	}

	constexpr GLenum textureKindToGLAttachment(Texture::Kind kind) noexcept{
		using Kind = Texture::Kind;
		switch(kind){
			case Kind::d16:
			case Kind::d32:
			case Kind::d32f:
				return GL_DEPTH_ATTACHMENT;

			case Kind::d24s8:
				return GL_DEPTH_STENCIL_ATTACHMENT;

			default: return GL_COLOR_ATTACHMENT0;
		}
	}
}

RenderFramebufferGL43::RenderFramebufferGL43(
	std::uint16_t w, std::uint16_t h, const std::vector<Texture::Kind> &attachments
)
	: m_w(w), m_h(h), m_attachments(attachments)
{
	m_texs.resize(m_attachments.size());

	glCreateFramebuffers(1, &m_handle);
	glCreateTextures(GL_TEXTURE_2D, m_attachments.size(), m_texs.data());

	std::uint16_t numColorAttachments = 0;

	for(std::size_t i = 0; i < m_texs.size(); i++){
		glTextureStorage2D(m_texs[i], 1, textureKindToGLFormat(m_attachments[i]), w, h);
		auto attachment = textureKindToGLAttachment(m_attachments[i]);
		if(attachment == GL_COLOR_ATTACHMENT0){
			attachment = (GLenum)((GLuint)attachment + numColorAttachments);
			++numColorAttachments;
		}

		glNamedFramebufferTexture(m_handle, attachment, m_texs[i], 0);
	}
}

RenderFramebufferGL43::~RenderFramebufferGL43(){
	glDeleteFramebuffers(1, &m_handle);
	glDeleteTextures(m_texs.size(), m_texs.data());
}

void RenderFramebufferGL43::use(Mode mode) noexcept{
	glBindFramebuffer(fbModeToGL(mode), m_handle);
}

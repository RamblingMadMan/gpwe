#include <array>

#include "gpwe/log.hpp"
#include "gpwe/Camera.hpp"
#include "gpwe/Shape.hpp"

#include "glbinding/glbinding.h"
#include "glbinding-aux/Meta.h"
#include "glbinding/gl43core/gl.h"

#include "glm/gtc/type_ptr.hpp"

#include "RendererGL43.hpp"

#include "shaders.hpp"

using namespace gpwe;
using namespace gl;

template<class Iter, class T>
inline Iter binary_find(Iter begin, Iter end, T val){
	Iter i = std::lower_bound(begin, end, val);

	if(i != end && !(val < *i)){
		return i;
	}
	else{
		return end;
	}
}

extern "C"
std::unique_ptr<gpwe::Renderer> gpweCreateRenderer_gl43(gpwe::GLGetProcFn getProcFn){
	return std::make_unique<gpwe::RendererGL43>(getProcFn);
}

struct DrawElementsIndirectCommand{
	GLuint count;
	GLuint primCount;
	GLuint firstIndex;
	GLuint baseVertex;
	GLuint baseInstance;
};

RenderGroupGL43::RenderGroupGL43(std::uint32_t numShapes, const Shape **shapes, std::uint32_t n)
	: m_numShapes(numShapes)
{
	glCreateVertexArrays(1, &m_vao);
	glCreateBuffers(std::size(m_bufs), m_bufs);

	std::vector<DrawElementsIndirectCommand> cmds;
	cmds.resize(numShapes);

	std::uint32_t totalNumPoints = 0, totalNumIndices = 0;

	for(std::uint32_t i = 0; i < numShapes; i++){
		auto shape = shapes[i];
		auto &cmd = cmds[i];

		cmd.count = shape->numIndices();
		cmd.primCount = n;
		cmd.firstIndex = totalNumIndices;
		cmd.baseVertex = totalNumPoints;
		cmd.baseInstance = 0;

		totalNumPoints += shape->numPoints();
		totalNumIndices += shape->numIndices();
	}

	std::vector<glm::vec3> verts, norms;
	std::vector<glm::vec2> uvs;
	std::vector<std::uint32_t> indices;

	verts.reserve(totalNumPoints);
	norms.reserve(totalNumPoints);
	uvs.reserve(totalNumPoints);
	indices.reserve(totalNumIndices);

	for(std::uint32_t i = 0; i < numShapes; i++){
		auto shape = shapes[i];

		verts.insert(verts.end(), shape->vertices(), shape->vertices() + shape->numPoints());
		norms.insert(norms.end(), shape->normals(), shape->normals() + shape->numPoints());
		uvs.insert(uvs.end(), shape->uvs(), shape->uvs() + shape->numPoints());
		indices.insert(indices.end(), shape->indices(), shape->indices() + shape->numIndices());
	}

	glNamedBufferStorage(m_bufs[0], sizeof(glm::vec3) * totalNumPoints, verts.data(), GL_MAP_READ_BIT);
	glNamedBufferStorage(m_bufs[1], sizeof(glm::vec3) * totalNumPoints, norms.data(), GL_MAP_READ_BIT);
	glNamedBufferStorage(m_bufs[2], sizeof(glm::vec2) * totalNumPoints, uvs.data(), GL_MAP_READ_BIT);
	glNamedBufferStorage(m_bufs[3], sizeof(std::uint32_t) * totalNumIndices, indices.data(), GL_MAP_READ_BIT);
	glNamedBufferStorage(m_bufs[4], sizeof(DrawElementsIndirectCommand) * numShapes, cmds.data(), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);

	m_cmdPtr = glMapNamedBufferRange(
		m_bufs[4],
		0, sizeof(DrawElementsIndirectCommand) * numShapes,
		GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT
	);

	glEnableVertexArrayAttrib(m_vao, 0);
	glEnableVertexArrayAttrib(m_vao, 1);
	glEnableVertexArrayAttrib(m_vao, 2);

	glVertexArrayAttribBinding(m_vao, 0, 0);
	glVertexArrayAttribBinding(m_vao, 1, 1);
	glVertexArrayAttribBinding(m_vao, 2, 2);

	constexpr GLintptr vertOffs[] = { 0, 0, 0 };
	constexpr GLsizei vertStrides[] = { sizeof(glm::vec3), sizeof(glm::vec3), sizeof(glm::vec2) };

	glVertexArrayVertexBuffers(m_vao, 0, 3, m_bufs, vertOffs, vertStrides);

	glVertexArrayAttribFormat(m_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribFormat(m_vao, 1, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribFormat(m_vao, 2, 2, GL_FLOAT, GL_FALSE, 0);

	glVertexArrayElementBuffer(m_vao, m_bufs[3]);
}

RenderGroupGL43::~RenderGroupGL43(){
	glDeleteBuffers(std::size(m_bufs), m_bufs);
	glDeleteVertexArrays(1, &m_vao);
}

void RenderGroupGL43::draw() const noexcept{
	glBindVertexArray(m_vao);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_bufs[4]);
	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, m_numShapes, sizeof(DrawElementsIndirectCommand));
}

void RenderGroupGL43::setNumInstances(std::uint32_t n){
	auto cmds = reinterpret_cast<DrawElementsIndirectCommand*>(m_cmdPtr);
	for(std::uint32_t i = 0; i < m_numShapes; i++){
		auto cmd = cmds + i;
		cmd->primCount = n;
		glFlushMappedNamedBufferRange(m_bufs[0], (i * sizeof(DrawElementsIndirectCommand)) + offsetof(DrawElementsIndirectCommand, primCount), sizeof(GLuint));
	}
}

std::uint32_t RenderGroupGL43::numInstances() const noexcept{
	auto cmd = reinterpret_cast<const DrawElementsIndirectCommand*>(m_cmdPtr);
	return cmd->primCount;
}

void *RenderGroupGL43::dataPtr(std::uint32_t idx){ return nullptr; }

void gpweGLMessageCB(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar *message,
	const void *userParam
){
	if(severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;

	logLn(
		severity == GL_DEBUG_SEVERITY_HIGH ? LogKind::error : LogKind::warning,
		"{}[{}]: {}",
		glbinding::aux::Meta::getString(source),
		glbinding::aux::Meta::getString(severity),
		std::string_view(message, length)
	);
}

RendererGL43::RendererGL43(GLGetProcFn getProcFn){
	log("{:<30}", "Initializing glbinding...");
	glbinding::initialize(getProcFn);
	logLn("Done");

#ifndef NDEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(gpweGLMessageCB, nullptr);
#endif

	log("{:<30}", "Creating framebuffer...");
	m_gbuffer = createFramebuffer(
		1280, 720,
		{
			Texture::Kind::d24s8, // depth+stencil
			Texture::Kind::rgba8, // diffuse/albedo
			Texture::Kind::rgb16f, // normals
			Texture::Kind::rgb16f // hdr/lighting
		}
	);
	logLn("Done");

	log("{:<30}", "Compiling shaders...");
	m_vertFullbright = createProgram(RenderProgram::Kind::vertex, vertFullbright);
	if(!m_vertFullbright){
		logError("Error\n");
		throw std::runtime_error("Error compiling fullbright vertex shader");
	}

	m_fragFullbright = createProgram(RenderProgram::Kind::fragment, fragFullbright);
	m_pipelineFullbright = createPipeline({ m_vertFullbright, m_fragFullbright });
	logLn("Done");

	logLn("");

	logLn("OpenGL Version: {}", (const char*)glGetString(GL_VERSION));
}

RendererGL43::~RendererGL43(){
}

void RendererGL43::present(const Camera *cam) noexcept{
	GLint curFb;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &curFb);

	GLint viewDims[4] = {0};
	glGetIntegerv(GL_VIEWPORT, viewDims);
	GLint oldW = viewDims[2];
	GLint oldH = viewDims[3];

	auto w = m_gbuffer->width();
	auto h = m_gbuffer->height();

	m_gbuffer->use();

	glViewport(0, 0, w, h);

	glFrontFace(GL_CCW);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	glClipControl(GL_UPPER_LEFT, GL_ZERO_TO_ONE);

	//glClearDepthf(0.f);
	//glDepthFunc(GL_GEQUAL);

	glClearColor(0.f, 0.f, 0.f, 1.f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	constexpr GLenum drawBufs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };

	glDrawBuffers(std::size(drawBufs), drawBufs);

	auto viewProj = cam->projMat() * cam->viewMat();

	auto fullbrightVert = reinterpret_cast<RenderProgramGL43*>(m_vertFullbright);

	auto viewProjLoc = glGetUniformLocation(fullbrightVert->handle(), "viewProj");
	if(viewProjLoc != -1){
		glProgramUniformMatrix4fv(fullbrightVert->handle(), viewProjLoc, 1, GL_FALSE, glm::value_ptr(viewProj));
	}

	m_pipelineFullbright->use();

	for(auto &&group : m_groups){
		group->draw();
	}

	m_gbuffer->use(RenderFramebuffer::Mode::read);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, curFb);

	glBlitFramebuffer(0, 0, w, h, 0, 0, oldW, oldH, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

std::unique_ptr<RenderGroup> RendererGL43::doCreateGroup(std::uint32_t numShapes, const Shape **shapes){
	return std::make_unique<RenderGroupGL43>(numShapes, shapes);
}

std::unique_ptr<RenderProgram> RendererGL43::doCreateProgram(RenderProgram::Kind kind, std::string_view src){
	return std::make_unique<RenderProgramGL43>(kind, src);
}

std::unique_ptr<RenderPipeline> RendererGL43::doCreatePipeline(const std::vector<RenderProgram*> &progs){
	std::vector<RenderProgramGL43*> glProgs;
	glProgs.reserve(progs.size());

	for(auto prog : progs){
		auto glProg = dynamic_cast<RenderProgramGL43*>(prog);
		if(!glProg){
			logLn("non-GL program given to createPipeline");
			return nullptr;
		}

		glProgs.emplace_back(glProg);
	}

	return std::make_unique<RenderPipelineGL43>(glProgs);
}

std::unique_ptr<RenderFramebuffer> RendererGL43::doCreateFramebuffer(
	std::uint16_t w, std::uint16_t h, const std::vector<Texture::Kind> &attachments
){
	return std::make_unique<RenderFramebufferGL43>(w, h, attachments);
}

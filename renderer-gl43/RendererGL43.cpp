#include <array>

#include "gpwe/log.hpp"
#include "gpwe/Camera.hpp"
#include "gpwe/Shape.hpp"

#include "glbinding/glbinding.h"
#include "glbinding-aux/Meta.h"
#include "glbinding/gl43core/gl.h"

#include "glm/gtc/type_ptr.hpp"

#include "RendererGL43.hpp"

#include "shaders/fullbright.vert.hpp"
#include "shaders/fullbright.frag.hpp"

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

GPWE_RENDER_PLUGIN(gpwe::RendererGL43, "OpenGL 4.3", "RamblingMad", 0, 0, 0)

struct DrawElementsIndirectCommand{
	GLuint count;
	GLuint primCount;
	GLuint firstIndex;
	GLuint baseVertex;
	GLuint baseInstance;
};

namespace {
	inline GLenum dataTypeToGL(render::DataType type){
		using Type = render::DataType;
		switch(type){
#define CASEMAP(a, b) case Type::a: return b
			CASEMAP(nat8, GL_UNSIGNED_BYTE);
			CASEMAP(nat16, GL_UNSIGNED_SHORT);
			CASEMAP(nat32, GL_UNSIGNED_INT);
			CASEMAP(int8, GL_BYTE);
			CASEMAP(int16, GL_SHORT);
			CASEMAP(int32, GL_INT);
			CASEMAP(float32, GL_FLOAT);
			CASEMAP(vec2, GL_FLOAT);
			CASEMAP(vec3, GL_FLOAT);
			CASEMAP(vec4, GL_FLOAT);
			CASEMAP(mat2, GL_FLOAT);
			CASEMAP(mat3, GL_FLOAT);
			CASEMAP(mat4, GL_FLOAT);
			default: return GL_INVALID_ENUM;
#undef CASEMAP
		}
	}
}

RenderGroupGL43::RenderGroupGL43(
	Vector<render::InstanceData> instDataInfo,
	std::uint32_t numShapes, const VertexShape **shapes
)
	: render::Group(std::move(instDataInfo))
	, m_numShapes(numShapes)
{
	std::size_t numAlloced =
	std::pow(2.0, std::ceil(std::log2(double(4))));

	m_numAllocated = numAlloced;

	glCreateBuffers(std::size(m_bufs), m_bufs);

	Vector<DrawElementsIndirectCommand> cmds;
	cmds.resize(numShapes);

	std::uint32_t totalNumPoints = 0, totalNumIndices = 0;

	for(std::uint32_t i = 0; i < numShapes; i++){
		auto shape = shapes[i];
		auto &cmd = cmds[i];

		cmd.count = shape->numIndices();
		cmd.primCount = 0; // start with no instances/
		cmd.firstIndex = totalNumIndices;
		cmd.baseVertex = totalNumPoints;
		cmd.baseInstance = 0;

		totalNumPoints += shape->numPoints();
		totalNumIndices += shape->numIndices();
	}

	Vector<Vec3> verts, norms;
	Vector<Vec2> uvs;
	Vector<std::uint32_t> indices;

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

	std::size_t totalAttribSize = instanceDataSize();

	glNamedBufferStorage(m_bufs[0], sizeof(Vec3) * totalNumPoints, verts.data(), GL_MAP_READ_BIT);
	glNamedBufferStorage(m_bufs[1], sizeof(Vec3) * totalNumPoints, norms.data(), GL_MAP_READ_BIT);
	glNamedBufferStorage(m_bufs[2], sizeof(Vec2) * totalNumPoints, uvs.data(), GL_MAP_READ_BIT);

	glNamedBufferStorage(m_bufs[3], sizeof(std::uint32_t) * totalNumIndices, indices.data(), GL_MAP_READ_BIT);

	glNamedBufferStorage(m_bufs[4], sizeof(DrawElementsIndirectCommand) * numShapes, cmds.data(), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);

	m_cmdPtr = glMapNamedBufferRange(
		m_bufs[4],
		0, sizeof(DrawElementsIndirectCommand) * numShapes,
		GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT
	);

	if(totalAttribSize > 0){
		glNamedBufferStorage(m_bufs[5], totalAttribSize * numAlloced, nullptr, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

		m_dataPtr = glMapNamedBufferRange(
			m_bufs[5],
			0, totalAttribSize * numAlloced,
			GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT
		);
	}

	glCreateVertexArrays(1, &m_vao);

	glEnableVertexArrayAttrib(m_vao, 0);
	glEnableVertexArrayAttrib(m_vao, 1);
	glEnableVertexArrayAttrib(m_vao, 2);

	glVertexArrayAttribBinding(m_vao, 0, 0);
	glVertexArrayAttribBinding(m_vao, 1, 1);
	glVertexArrayAttribBinding(m_vao, 2, 2);

	constexpr GLintptr vertOffs[] = { 0, 0, 0 };
	constexpr GLsizei vertStrides[] = { sizeof(Vec3), sizeof(Vec3), sizeof(Vec2) };

	glVertexArrayVertexBuffers(m_vao, 0, 3, m_bufs, vertOffs, vertStrides);

	glVertexArrayAttribFormat(m_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribFormat(m_vao, 1, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribFormat(m_vao, 2, 2, GL_FLOAT, GL_FALSE, 0);

	if(totalAttribSize > 0){
		glVertexArrayVertexBuffer(m_vao, 3, m_bufs[5], 0, totalAttribSize);
		glVertexArrayBindingDivisor(m_vao, 3, 1);

		std::uint32_t curAttrOff = 0;

		for(GLuint i = 0; i < instanceDataInfo().size(); i++){
			auto &&info = instanceDataInfo()[i];

			GLuint attribIdx = 3 + i;

			glEnableVertexArrayAttrib(m_vao, attribIdx);
			glVertexArrayAttribBinding(m_vao, attribIdx, 3);
			glVertexArrayAttribFormat(m_vao, attribIdx, render::dataTypeNumComponents(info.type()), dataTypeToGL(info.type()), GL_FALSE, curAttrOff);

			curAttrOff += info.size();
		}
	}

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

void *RenderGroupGL43::dataPtr(std::uint32_t idx){
	if(idx >= numManaged<render::Instance>()) return nullptr;
	return reinterpret_cast<char*>(m_dataPtr) + (instanceDataSize() * idx);
}

UniquePtr<render::Instance> RenderGroupGL43::doCreateInstance(){
	std::size_t totalAttribSize = instanceDataSize();

	if(totalAttribSize > 0 && numManaged<render::Instance>() == m_numAllocated){
		auto newAlloced = m_numAllocated * 2;

		GLuint newAttribBuf;
		glCreateBuffers(1, &newAttribBuf);

		glNamedBufferStorage(
			newAttribBuf,
			totalAttribSize * newAlloced,
			nullptr,
			GL_MAP_READ_BIT | GL_MAP_WRITE_BIT |
			GL_MAP_PERSISTENT_BIT |
			GL_MAP_COHERENT_BIT
		);

		glCopyNamedBufferSubData(
			m_bufs[5], newAttribBuf,
			0, 0,
			m_numAllocated * totalAttribSize
		);

		auto oldBuf = m_bufs[5];

		m_bufs[5] = newAttribBuf;

		m_dataPtr = glMapNamedBufferRange(
			m_bufs[5],
			0, totalAttribSize * newAlloced,
			GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT
		);

		m_numAllocated = newAlloced;

		glVertexArrayVertexBuffer(m_vao, 3, m_bufs[5], 0, totalAttribSize);
		glVertexArrayBindingDivisor(m_vao, 3, 1);

		std::uint32_t curAttrOff = 0;

		for(GLuint i = 0; i < instanceDataInfo().size(); i++){
			auto &&info = instanceDataInfo()[i];

			GLuint attribIdx = 3 + i;

			glEnableVertexArrayAttrib(m_vao, attribIdx);
			glVertexArrayAttribBinding(m_vao, attribIdx, 3);
			glVertexArrayAttribFormat(m_vao, attribIdx, render::dataTypeNumComponents(info.type()), dataTypeToGL(info.type()), GL_FALSE, curAttrOff);

			curAttrOff += info.size();
		}

		glDeleteBuffers(1, &oldBuf);
	}

	auto cmds = reinterpret_cast<DrawElementsIndirectCommand*>(m_cmdPtr);
	for(std::uint32_t i = 0; i < m_numShapes; i++){
		auto cmd = cmds + i;
		++cmd->primCount;
		glFlushMappedNamedBufferRange(m_bufs[4], (i * sizeof(DrawElementsIndirectCommand)) + offsetof(DrawElementsIndirectCommand, primCount), sizeof(GLuint));
	}

	return makeUnique<RenderInstanceGL43>(this, (std::uint32_t)numManaged<render::Instance>());
}

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

	log::outLn(
		severity == GL_DEBUG_SEVERITY_HIGH ? log::Kind::error : log::Kind::warning,
		"{}[{}]: {}",
		glbinding::aux::Meta::getString(source),
		glbinding::aux::Meta::getString(severity),
		std::string_view(message, length)
	);
}

RendererGL43::RendererGL43(){}

RendererGL43::~RendererGL43(){}

void RendererGL43::init(){
	log::info("{:<30}", "Initializing glbinding...");
	glbinding::initialize(reinterpret_cast<GLGetProcFn>(m_arg));
	log::infoLn("Done");

#ifndef NDEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(gpweGLMessageCB, nullptr);
#endif

	log::info("{:<30}", "Creating framebuffer...");
	m_gbuffer = create<render::Framebuffer>(
		1280, 720,
		Vector<render::TextureKind>{
			render::TextureKind::d24s8, // depth+stencil
			render::TextureKind::rgba8, // diffuse/albedo
			render::TextureKind::rgb16f, // normals
			render::TextureKind::rgb10a2 // hdr/lighting
		}
	);
	log::infoLn("Done");

	log::info("{:<30}", "Compiling shaders...");

	m_vertFullbright = create<render::Program>(render::ProgramKind::vertex, embed::shaders_fullbright_vert_str());
	if(!m_vertFullbright){
		log::error("Error\n");
		throw std::runtime_error("Error compiling fullbright vertex shader");
	}

	m_fragFullbright = create<render::Program>(render::ProgramKind::fragment, embed::shaders_fullbright_frag_str());
	if(!m_vertFullbright){
		log::error("Error\n");
		throw std::runtime_error("Error compiling fullbright fragment shader");
	}

	m_pipelineFullbright = create<render::Pipeline>(Vector<render::Program*>{ m_vertFullbright, m_fragFullbright });
	log::infoLn("Done");

	log::infoLn("");

	log::infoLn("OpenGL Version: {}", (const char*)glGetString(GL_VERSION));

	log::infoLn("");
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
	glEnable(GL_CULL_FACE);

	glClipControl(GL_UPPER_LEFT, GL_ZERO_TO_ONE);

	//glClearDepthf(0.f);
	//glDepthFunc(GL_GEQUAL);

	glClearColor(0.f, 0.f, 0.f, 1.f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	constexpr GLenum drawBufs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };

	glDrawBuffers(std::size(drawBufs), drawBufs);

	Vec4 colorMix = Vec4(1.f);

	auto viewProj = cam->projMat() * cam->viewMat();

	auto fullbrightVert = reinterpret_cast<RenderProgramGL43*>(m_vertFullbright);
	auto fullbrightFrag = reinterpret_cast<RenderProgramGL43*>(m_fragFullbright);

	auto viewProjLoc = glGetUniformLocation(fullbrightVert->handle(), "viewProj");
	if(viewProjLoc != -1){
		glProgramUniformMatrix4fv(fullbrightVert->handle(), viewProjLoc, 1, GL_FALSE, glm::value_ptr(viewProj));
	}

	auto colorMixLoc = glGetUniformLocation(fullbrightFrag->handle(), "colorMix");
	if(colorMixLoc != -1){
		glProgramUniform4fv(fullbrightFrag->handle(), colorMixLoc, 1, glm::value_ptr(colorMix));
	}

	m_pipelineFullbright->use();

	for(auto &&group : managed<render::Group>()){
		group->draw();
	}

	m_gbuffer->use(render::Framebuffer::Mode::read);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, curFb);

	glViewport(0, 0, oldW, oldH);

	glBlitFramebuffer(0, 0, w, h, 0, 0, oldW, oldH, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

UniquePtr<render::Group> RendererGL43::doCreateGroup(
	std::uint32_t numShapes, const VertexShape **shapes,
	Vector<render::InstanceData> instanceDataInfo
){
	return makeUnique<RenderGroupGL43>(std::move(instanceDataInfo), numShapes, shapes);
}

UniquePtr<render::Program> RendererGL43::doCreateProgram(render::ProgramKind kind, std::string_view src){
	return makeUnique<RenderProgramGL43>(kind, src);
}

UniquePtr<render::Pipeline> RendererGL43::doCreatePipeline(const Vector<render::Program*> &progs){
	Vector<RenderProgramGL43*> glProgs;
	glProgs.reserve(progs.size());

	for(auto prog : progs){
		auto glProg = dynamic_cast<RenderProgramGL43*>(prog);
		if(!glProg){
			log::warnLn("non-GL program given to createPipeline");
			return nullptr;
		}

		glProgs.emplace_back(glProg);
	}

	return makeUnique<RenderPipelineGL43>(glProgs);
}

UniquePtr<render::Framebuffer> RendererGL43::doCreateFramebuffer(
	std::uint16_t w, std::uint16_t h, const Vector<render::TextureKind> &attachments
){
	return makeUnique<RenderFramebufferGL43>(w, h, attachments);
}

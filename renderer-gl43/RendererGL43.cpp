#include <array>

#include "gpwe/log.hpp"

#include "glbinding/glbinding.h"
#include "glbinding/gl43core/gl.h"

#include "RendererGL43.hpp"

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

RenderGroupGL43::RenderGroupGL43(const Shape *shape){
	glCreateVertexArrays(1, &m_vao);
	glCreateBuffers(std::size(m_bufs), m_bufs);
}

RenderGroupGL43::~RenderGroupGL43(){
	glDeleteBuffers(std::size(m_bufs), m_bufs);
	glDeleteVertexArrays(1, &m_vao);
}

void RenderGroupGL43::draw() const noexcept{}
void RenderGroupGL43::setNumInstances(std::uint32_t n){}

void *RenderGroupGL43::dataPtr(std::uint32_t idx){ return nullptr; }

RendererGL43::RendererGL43(GLGetProcFn getProcFn){
	log("{:<30}", "Initializing glbinding...");
	glbinding::initialize(getProcFn);
	log("Done\n");
}

RendererGL43::~RendererGL43(){
	for(auto group : m_groups){
		delete group;
	}
}

void RendererGL43::present() noexcept{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for(auto group : m_groups){
		group->draw();
	}
}

RenderGroupGL43 *RendererGL43::createGroup(const Shape *shape){
	// might want to move away from bare new
	auto group = new RenderGroupGL43(shape);
	auto it = std::upper_bound(m_groups.begin(), m_groups.end(), group);
	m_groups.insert(it, group);
	return group;
}

bool RendererGL43::destroyGroup(RenderGroup *group){
	auto res = binary_find(m_groups.begin(), m_groups.end(), group);
	if(res != m_groups.end()){
		delete *res;
		m_groups.erase(res);
		return true;
	}

	return false;
}

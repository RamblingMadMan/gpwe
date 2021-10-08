#ifndef GPWE_CLIENT_RENDERERGL43_HPP
#define GPWE_CLIENT_RENDERERGL43_HPP 1

#include "gpwe/Renderer.hpp"

#include "plf_list.h"

namespace gpwe{
	class RenderGroupGL43: public RenderGroup{
		public:
			~RenderGroupGL43();

			void draw() const noexcept override;
			void setNumInstances(std::uint32_t n) override;

		protected:
			void *dataPtr(std::uint32_t idx) override;

		private:
			RenderGroupGL43(const Shape *shape);

			std::uint32_t m_vao;
			std::uint32_t m_bufs[4];

			friend class RendererGL43;
	};

	using GLProc = void(*)();
	using GLGetProcFn = GLProc(*)(const char*);

	class RendererGL43: public Renderer{
		public:
			RendererGL43(GLGetProcFn getProcFn);
			~RendererGL43();

			void present() noexcept override;

			RenderGroupGL43 *createGroup(const Shape *shape) override;
			bool destroyGroup(RenderGroup *group) override;

		private:
			plf::list<RenderGroupGL43*> m_groups;
	};
}

extern "C"
std::unique_ptr<gpwe::Renderer> gpweCreateRenderer_gl43(gpwe::GLGetProcFn getProcFn);

#endif // !GPWE_CLIENT_RENDERERGL_HPP

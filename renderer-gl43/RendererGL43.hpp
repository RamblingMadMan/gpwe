#ifndef GPWE_RENDERER_GL43_HPP
#define GPWE_RENDERER_GL43_HPP 1

#include "gpwe/render.hpp"

namespace gpwe{
	class RenderGroupGL43: public render::Group{
		public:
			explicit RenderGroupGL43(std::uint32_t numShapes, const VertexShape **shapes, std::uint32_t n = 1);
			~RenderGroupGL43();

			void draw() const noexcept override;
			void setNumInstances(std::uint32_t n) override;

			std::uint32_t numInstances() const noexcept override;

		protected:
			void *dataPtr(std::uint32_t idx) override;

		private:
			std::uint32_t m_numShapes;
			std::uint32_t m_vao;
			std::uint32_t m_bufs[5];
			void *m_cmdPtr;

			friend class RendererGL43;
	};

	class RenderProgramGL43: public render::Program{
		public:
			RenderProgramGL43(Kind kind_, std::string_view src);
			~RenderProgramGL43();

			Kind kind() const noexcept override{ return m_kind; }

			std::uint32_t handle() const noexcept{ return m_handle; }

		private:
			Kind m_kind;
			std::uint32_t m_handle;

			friend class RendererGL43;
	};

	class RenderPipelineGL43: public render::Pipeline{
		public:
			RenderPipelineGL43(const Vector<RenderProgramGL43*> &progs);
			~RenderPipelineGL43();

			void use() const noexcept override;

			std::uint32_t handle() const noexcept{ return m_handle; }

		private:
			std::uint32_t m_handle;
			Vector<RenderProgramGL43*> m_progs;
	};

	class RenderFramebufferGL43: public render::Framebuffer{
		public:
			RenderFramebufferGL43(std::uint16_t w, std::uint16_t h, const Vector<render::Texture::Kind> &attachments);
			~RenderFramebufferGL43();

			void use(Mode mode) noexcept override;

			std::uint16_t width() const noexcept override{ return m_w; }
			std::uint16_t height() const noexcept override{ return m_h; }
			std::uint32_t handle() const noexcept{ return m_handle; }

			std::uint32_t numAttachments() const noexcept override{ return m_attachments.size(); }
			render::TextureKind attachmentKind(std::uint32_t idx) const noexcept override{ return m_attachments[idx]; }
			std::uint32_t attachmentHandle(std::uint32_t idx) const noexcept{ return m_texs[idx]; }

		private:
			std::uint16_t m_w, m_h;
			std::uint32_t m_handle;
			Vector<std::uint32_t> m_texs;
			Vector<render::TextureKind> m_attachments;
	};

	using GLProc = void(*)();
	using GLGetProcFn = GLProc(*)(const char*);

	class RendererGL43: public render::Manager{
		public:
			RendererGL43();
			~RendererGL43();

			void init() override;

			void present(const Camera *cam) noexcept override;

		protected:
			UniquePtr<render::Group> doCreateGroup(std::uint32_t numShapes, const VertexShape **shapes) override;

			UniquePtr<render::Texture> doCreateTexture(std::uint16_t w, std::uint16_t h, render::TextureKind, const void *pixels) override{
				return nullptr;
			}

			UniquePtr<render::Program> doCreateProgram(render::ProgramKind kind, std::string_view src) override;

			UniquePtr<render::Pipeline> doCreatePipeline(const Vector<render::Program*> &progs) override;

			UniquePtr<render::Framebuffer> doCreateFramebuffer(
				std::uint16_t w, std::uint16_t h, const Vector<render::TextureKind> &attachments
			) override;

		private:
			render::Framebuffer *m_gbuffer;
			render::Program *m_vertFullbright, *m_fragFullbright;
			render::Pipeline *m_pipelineFullbright;
	};
}

#endif // !GPWE_RENDERER_GL43_HPP

#ifndef GPWE_RENDERER_GL43_HPP
#define GPWE_RENDERER_GL43_HPP 1

#include "gpwe/Renderer.hpp"

namespace gpwe{
	class RenderGroupGL43: public RenderGroup{
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

	class RenderProgramGL43: public RenderProgram{
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

	class RenderPipelineGL43: public RenderPipeline{
		public:
			RenderPipelineGL43(const Vector<RenderProgramGL43*> &progs);
			~RenderPipelineGL43();

			void use() const noexcept override;

			std::uint32_t handle() const noexcept{ return m_handle; }

		private:
			std::uint32_t m_handle;
			Vector<RenderProgramGL43*> m_progs;
	};

	class RenderFramebufferGL43: public RenderFramebuffer{
		public:
			RenderFramebufferGL43(std::uint16_t w, std::uint16_t h, const Vector<Texture::Kind> &attachments);
			~RenderFramebufferGL43();

			void use(Mode mode) noexcept override;

			std::uint16_t width() const noexcept override{ return m_w; }
			std::uint16_t height() const noexcept override{ return m_h; }
			std::uint32_t handle() const noexcept{ return m_handle; }

			std::uint32_t numAttachments() const noexcept override{ return m_attachments.size(); }
			Texture::Kind attachmentKind(std::uint32_t idx) const noexcept override{ return m_attachments[idx]; }
			std::uint32_t attachmentHandle(std::uint32_t idx) const noexcept{ return m_texs[idx]; }

		private:
			std::uint16_t m_w, m_h;
			std::uint32_t m_handle;
			Vector<std::uint32_t> m_texs;
			Vector<Texture::Kind> m_attachments;
	};

	using GLProc = void(*)();
	using GLGetProcFn = GLProc(*)(const char*);

	class RendererGL43: public Renderer{
		public:
			RendererGL43(void *getProcFn);
			~RendererGL43();

			void present(const Camera *cam) noexcept override;

		protected:
			UniquePtr<RenderGroup> doCreateGroup(std::uint32_t numShapes, const VertexShape **shapes) override;

			UniquePtr<RenderProgram> doCreateProgram(RenderProgram::Kind kind, std::string_view src) override;

			UniquePtr<RenderPipeline> doCreatePipeline(const Vector<RenderProgram*> &progs) override;

			UniquePtr<RenderFramebuffer> doCreateFramebuffer(
				std::uint16_t w, std::uint16_t h, const Vector<Texture::Kind> &attachments
			) override;

		private:
			RenderFramebuffer *m_gbuffer;
			RenderProgram *m_vertFullbright, *m_fragFullbright;
			RenderPipeline *m_pipelineFullbright;
	};
}

#endif // !GPWE_RENDERER_GL43_HPP

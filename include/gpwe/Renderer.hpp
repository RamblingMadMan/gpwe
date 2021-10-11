#ifndef GPWE_RENDERER_HPP
#define GPWE_RENDERER_HPP 1

#include <cstdint>

#include "Version.hpp"
#include "Vector.hpp"
#include "List.hpp"
#include "Texture.hpp"

namespace gpwe{
	class Camera;
	class VertexShape;

	class RenderGroup{
		public:
			virtual ~RenderGroup() = default;

			virtual void draw() const noexcept = 0;
			virtual void setNumInstances(std::uint32_t n) = 0;

			virtual std::uint32_t numInstances() const noexcept = 0;

		protected:
			virtual void *dataPtr(std::uint32_t idx) = 0;
	};

	class RenderProgram{
		public:
			enum class Kind{
				vertex,
				geometry,
				fragment,
				compute,

				count
			};

			virtual ~RenderProgram() = default;

			virtual Kind kind() const noexcept = 0;
	};

	class RenderPipeline{
		public:
			virtual ~RenderPipeline() = default;

			virtual void use() const noexcept = 0;
	};

	class RenderFramebuffer{
		public:
			enum class Mode{
				write, read, readWrite,
				count
			};

			virtual ~RenderFramebuffer() = default;

			virtual std::uint16_t width() const noexcept = 0;
			virtual std::uint16_t height() const noexcept = 0;

			virtual void use(Mode mode = Mode::write) noexcept = 0;

			virtual std::uint32_t numAttachments() const noexcept = 0;
			virtual Texture::Kind attachmentKind(std::uint32_t idx) const noexcept = 0;
	};

	class Renderer{
		public:
			virtual ~Renderer() = default;

			virtual void present(const Camera *cam) noexcept = 0;

			RenderGroup *createGroup(std::uint32_t numShapes, const VertexShape **shapes);
			RenderGroup *createGroup(const VertexShape *shape){ return createGroup(1, &shape); }
			bool destroyGroup(RenderGroup *group);

			RenderProgram *createProgram(RenderProgram::Kind kind, std::string_view src);
			bool destroyProgram(RenderProgram *program);

			RenderPipeline *createPipeline(const Vector<RenderProgram*> &programs);
			bool destroyPipeline(RenderPipeline *pipeline);

			RenderFramebuffer *createFramebuffer(std::uint16_t w, std::uint16_t h, const Vector<Texture::Kind> &attachments);
			bool destroyFramebuffer(RenderFramebuffer *fb);

		protected:
			virtual UniquePtr<RenderGroup> doCreateGroup(std::uint32_t numShapes, const VertexShape **shapes) = 0;
			virtual UniquePtr<RenderProgram> doCreateProgram(RenderProgram::Kind kind, std::string_view src) = 0;
			virtual UniquePtr<RenderPipeline> doCreatePipeline(const Vector<RenderProgram*> &progs) = 0;
			virtual UniquePtr<RenderFramebuffer> doCreateFramebuffer(std::uint16_t w, std::uint16_t h, const Vector<Texture::Kind> &attachments) = 0;

			List<UniquePtr<RenderGroup>> m_groups;
			List<UniquePtr<RenderProgram>> m_progs;
			List<UniquePtr<RenderPipeline>> m_pipelines;
			List<UniquePtr<RenderFramebuffer>> m_fbs;
	};
}

#define GPWE_RENDERER(type, name, author, major, minor, patch)\
extern "C" const char *gpweRendererName(){ return name; }\
extern "C" const char *gpweRendererAuthor(){ return author; }\
extern "C" gpwe::Version gpweRendererVersion(){ return { major, minor, patch }; }\
extern "C" gpwe::UniquePtr<gpwe::Renderer> gpweCreateRenderer(void *param){ return gpwe::makeUnique<type>(param); }

#endif // !GPWE_RENDERER_HPP

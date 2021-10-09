#ifndef GPWE_RENDERER_HPP
#define GPWE_RENDERER_HPP 1

#include <cstdint>
#include <vector>
#include <string_view>

#include "plf_list.h"

#include "Texture.hpp"

namespace gpwe{
	class Camera;
	class Shape;

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

			RenderGroup *createGroup(std::uint32_t numShapes, const Shape **shapes);
			RenderGroup *createGroup(const Shape *shape){ return createGroup(1, &shape); }
			bool destroyGroup(RenderGroup *group);

			RenderProgram *createProgram(RenderProgram::Kind kind, std::string_view src);
			bool destroyProgram(RenderProgram *program);

			RenderPipeline *createPipeline(const std::vector<RenderProgram*> &programs);
			bool destroyPipeline(RenderPipeline *pipeline);

			RenderFramebuffer *createFramebuffer(std::uint16_t w, std::uint16_t h, const std::vector<Texture::Kind> &attachments);
			bool destroyFramebuffer(RenderFramebuffer *fb);

		protected:
			virtual std::unique_ptr<RenderGroup> doCreateGroup(std::uint32_t numShapes, const Shape **shapes) = 0;
			virtual std::unique_ptr<RenderProgram> doCreateProgram(RenderProgram::Kind kind, std::string_view src) = 0;
			virtual std::unique_ptr<RenderPipeline> doCreatePipeline(const std::vector<RenderProgram*> &progs) = 0;
			virtual std::unique_ptr<RenderFramebuffer> doCreateFramebuffer(std::uint16_t w, std::uint16_t h, const std::vector<Texture::Kind> &attachments) = 0;

			plf::list<std::unique_ptr<RenderGroup>> m_groups;
			plf::list<std::unique_ptr<RenderProgram>> m_progs;
			plf::list<std::unique_ptr<RenderPipeline>> m_pipelines;
			plf::list<std::unique_ptr<RenderFramebuffer>> m_fbs;
	};
}

#endif // !GPWE_RENDERER_HPP

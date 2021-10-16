#ifndef GPWE_RENDER_HPP
#define GPWE_RENDER_HPP 1

#include "Version.hpp"
#include "Manager.hpp"
#include "Vector.hpp"
#include "Shape.hpp"
#include "Texture.hpp"
#include "Camera.hpp"

namespace gpwe::render{
	class Texture;
	class Group;
	class Framebuffer;
	class Program;
	class Pipeline;

	enum class TextureKind{
		// Color formats
		r8, rg8, rgb8, rgba8,
		r16, rg16, rgb16, rgba16,
		r16n, rg16n, rgb16n, rgba16n,
		r16i, rg16i, rgb16i, rgba16i,
		r16f, rg16f, rgb16f, rgba16f,
		r32n, rg32n, rgb32n, rgba32n,
		r32i, rg32i, rgb32i, rgba32i,
		r32f, rg32f, rgb32f, rgba32f,

		// Special formats
		rgb10a2,

		// Depth(+Stencil) formats
		d16,
		d32, d32f,
		d24s8,

		count
	};

	enum class ProgramKind{
		vertex, fragment, geometry, compute,
		count
	};

	class Manager:
			public gpwe::Manager<
				Manager, ManagerKind::render,
				Group, Texture, Framebuffer, Program, Pipeline
			>
	{
		public:
			virtual ~Manager() = default;

			virtual void present(const Camera *cam) noexcept = 0;

			void setArg(void *arg) noexcept{ m_arg = arg; }

			Group *createGroup(const VertexShape *shape){
				return create<Group>(1, &shape);
			}

			void setRenderSize(std::uint16_t w, std::uint16_t h){
				onRenderResize(w, h);
				m_w = w;
				m_h = h;
			}

			std::uint16_t renderWidth() const noexcept{ return m_w; }
			std::uint16_t renderHeight() const noexcept{ return m_h; }

		protected:
			virtual UniquePtr<Group> doCreateGroup(
				std::uint32_t numShapes, const VertexShape **shapes
			) = 0;

			virtual UniquePtr<Texture> doCreateTexture(
				std::uint16_t w, std::uint16_t h, TextureKind kind,
				const void *pixels = nullptr
			) = 0;

			virtual UniquePtr<Framebuffer> doCreateFramebuffer(
				std::uint16_t w, std::uint16_t h,
				const Vector<TextureKind> &attachments
			) = 0;

			virtual UniquePtr<Program> doCreateProgram(ProgramKind kind, std::string_view src) = 0;

			virtual UniquePtr<Pipeline> doCreatePipeline(const Vector<Program*> &progs) = 0;

			virtual void onRenderResize(std::uint16_t w, std::uint16_t h){}

			void *m_arg = nullptr;

			std::uint16_t m_w = 0, m_h = 0;

			friend class Group;
			friend class Texture;
			friend class Framebuffer;
			friend class Program;
			friend class Pipeline;
	};

	class Group: public gpwe::Managed<Manager, &Manager::doCreateGroup>{
		public:
			virtual ~Group() = default;

			virtual void draw() const noexcept = 0;
			virtual void setNumInstances(std::uint32_t n) = 0;

			virtual std::uint32_t numInstances() const noexcept = 0;

		protected:
			virtual void *dataPtr(std::uint32_t idx) = 0;
	};

	class Texture: public gpwe::Managed<Manager, &Manager::doCreateTexture>{
		public:
			using Kind = TextureKind;

			virtual ~Texture() = default;
	};

	class Framebuffer: public gpwe::Managed<Manager, &Manager::doCreateFramebuffer>{
		public:
			enum class Mode{
				write, read, readWrite,
				count
			};

			virtual ~Framebuffer() = default;

			virtual std::uint16_t width() const noexcept = 0;
			virtual std::uint16_t height() const noexcept = 0;

			virtual void use(Mode mode = Mode::write) noexcept = 0;

			virtual std::uint32_t numAttachments() const noexcept = 0;
			virtual Texture::Kind attachmentKind(std::uint32_t idx) const noexcept = 0;
	};

	class Program: public gpwe::Managed<Manager, &Manager::doCreateProgram>{
		public:
			using Kind = ProgramKind;

			virtual ~Program() = default;

			virtual Kind kind() const noexcept = 0;
	};

	class Pipeline: public gpwe::Managed<Manager, &Manager::doCreatePipeline>{
		public:
			virtual ~Pipeline() = default;

			virtual void use() const noexcept = 0;
	};
}

#define GPWE_RENDERER(type, name, author, major, minor, patch)\
	GPWE_PLUGIN(render, type, name, author, major, minor, patch)

#endif // !GPWE_RENDER_HPP

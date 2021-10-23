#ifndef GPWE_RENDER_HPP
#define GPWE_RENDER_HPP 1

#include "util/Vector.hpp"

#include "Version.hpp"
#include "Manager.hpp"
#include "Shape.hpp"
#include "Camera.hpp"

namespace gpwe::render{
	class Texture;
	class Group;
	class Framebuffer;
	class Program;
	class Pipeline;
	class Pass;

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

	enum class DataType{
		nat8, nat16, nat32,
		int8, int16, int32,
		float32,

		vec2, vec3, vec4,
		mat2, mat3, mat4,

		count
	};

	inline std::size_t dataTypeSize(DataType type) noexcept{
		using Type = DataType;
		switch(type){
		#define CASEMAP(a, b) case Type::a: return b
			CASEMAP(nat8, 1);
			CASEMAP(nat16, 2);
			CASEMAP(nat32, 4);
			CASEMAP(int8, 1);
			CASEMAP(int16, 2);
			CASEMAP(int32, 4);
			CASEMAP(float32, 4);
			CASEMAP(vec2, sizeof(Vec2));
			CASEMAP(vec3, sizeof(Vec3));
			CASEMAP(vec4, sizeof(Vec4));
			CASEMAP(mat2, sizeof(Mat2));
			CASEMAP(mat3, sizeof(Mat3));
			CASEMAP(mat4, sizeof(Mat4));
			default: return 0;
		#undef CASEMAP
		}
	}

	inline std::size_t dataTypeNumComponents(DataType type) noexcept{
		using Type = DataType;
		switch(type){
		#define CASEMAP(a, b) case Type::a: return b
			CASEMAP(nat8, 1);
			CASEMAP(nat16, 1);
			CASEMAP(nat32, 1);
			CASEMAP(int8, 1);
			CASEMAP(int16, 1);
			CASEMAP(int32, 1);
			CASEMAP(float32, 1);
			CASEMAP(vec2, 2);
			CASEMAP(vec3, 3);
			CASEMAP(vec4, 4);
			CASEMAP(mat2, 2*2);
			CASEMAP(mat3, 3*3);
			CASEMAP(mat4, 4*4);
			default: return 0;
		#undef CASEMAP
		}
	}

	class InstanceData: public Object<InstanceData>{
		public:
			DataType type() const noexcept{ return m_type; }
			std::size_t size() const noexcept{ return m_len * dataTypeSize(m_type); }
			std::uint32_t len() const noexcept{ return m_len; }

		protected:
			InstanceData(DataType type_, std::uint32_t len_ = 1) noexcept
				: m_type(type_), m_len(len_){}

		private:
			DataType m_type;
			std::uint32_t m_len;
	};

	class Manager:
			public Object<Manager>,

			public gpwe::Manager<
				Manager, ManagerKind::render,
				Group, Texture, Framebuffer, Program, Pipeline
			>
	{
		public:
			virtual ~Manager() = default;

			virtual void present(const Camera *cam) noexcept = 0;

			void setArg(void *arg) noexcept{ m_arg = arg; }

			Group *createGroup(
				const VertexShape *shape,
				Vector<InstanceData> instanceDataInfo = {}
			){
				return create<Group>(1, &shape, std::move(instanceDataInfo));
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
				std::uint32_t numShapes, const VertexShape **shapes,
				Vector<InstanceData> instanceDataInfo = {}
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

	class Instance;

	class Group:
			public gpwe::Managed<Group, &Manager::doCreateGroup>,
			public gpwe::Manager<Group, ManagerKind::data, Instance>
	{
		public:
			virtual ~Group() = default;

			virtual void draw() const noexcept = 0;

			std::size_t instanceDataSize() const noexcept{
				return m_totalInstanceDataSize;
			}

			const Vector<InstanceData> &instanceDataInfo() const noexcept{
				return m_instanceDataInfo;
			}

		protected:
			Group(Vector<InstanceData> dataInfo = {}) noexcept
				: m_instanceDataInfo(std::move(dataInfo))
				, m_totalInstanceDataSize(0)
			{
				for(auto &&info : m_instanceDataInfo){
					m_totalInstanceDataSize += info.size();
				}
			}

			virtual UniquePtr<Instance> doCreateInstance() = 0;
			virtual void *dataPtr(std::uint32_t idx) = 0;

		private:
			Vector<InstanceData> m_instanceDataInfo;
			std::size_t m_totalInstanceDataSize;

			friend class Instance;
	};

	class Instance: public gpwe::Managed<Instance, &Group::doCreateInstance>{
		public:
			Group *group() noexcept{ return m_group; }
			const Group *group() const noexcept{ return m_group; }

			std::uint32_t index() const noexcept{ return m_idx; }

		protected:
			Instance(Group *group_, std::uint32_t idx_)
				: m_group(group_), m_idx(idx_){}

			Group *m_group;
			std::uint32_t m_idx;
	};

	class Texture: public gpwe::Managed<Texture, &Manager::doCreateTexture>{
		public:
			using Kind = TextureKind;

			virtual ~Texture() = default;
	};

	class Framebuffer: public gpwe::Managed<Framebuffer, &Manager::doCreateFramebuffer>{
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

	class Program: public gpwe::Managed<Program, &Manager::doCreateProgram>{
		public:
			using Kind = ProgramKind;

			virtual ~Program() = default;

			virtual Kind kind() const noexcept = 0;
	};

	class Pipeline:
		public gpwe::Managed<
			Pipeline,
			&Manager::doCreatePipeline
		>
	{
		public:
			virtual ~Pipeline() = default;

			virtual void use() const noexcept = 0;
	};

	class Pass{
		public:
			virtual ~Pass() = default;
	};
}

#define GPWE_RENDER_PLUGIN(type, name, author, major, minor, patch)\
	GPWE_PLUGIN(render, type, name, author, major, minor, patch)

#endif // !GPWE_RENDER_HPP

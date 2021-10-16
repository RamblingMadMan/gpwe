#ifndef GPWE_RESOURCE_HPP
#define GPWE_RESOURCE_HPP 1

#include <memory>
#include <filesystem>
#include <variant>

#include "Manager.hpp"
#include "Str.hpp"
#include "Map.hpp"
#include "Shape.hpp"
#include "Texture.hpp"

namespace gpwe{
	namespace fs = std::filesystem;

	using Path = fs::path;
}

namespace gpwe::resource{
	class Asset;
	class Plugin;
	class Image;
	class Font;
	class Model;

	enum class Access: std::uint8_t{
		read = 0x1, write = 0x2, readWrite = read | write
	};

	class Manager{
		public:
			Manager();
			~Manager();

			bool mount(const Path &path, StrView dir, bool mountBefore = true);

			Asset *openFile(StrView path, Access access_ = Access::read);
			Plugin *openPlugin(StrView path);
			Font *openFont(StrView path);
			Model *openModel(StrView path);

			void update();

			Vector<Plugin*> plugins() const{
				Vector<Plugin*> ret;
				ret.reserve(m_assets.size());
				for(auto &&p : m_plugins){
					ret.emplace_back(p.second);
				}
				return ret;
			}

		private:
			UniquePtr<Model> createModelFileAsset(
				Str path,
				Vector<char> bytes
			);

			UniquePtr<Font> createFontFileAsset(
				Str path,
				Vector<char> bytes
			);

			UniquePtr<Plugin> createPluginFileAsset(Str path);

			Map<Str, Str> m_mounts;
			Vector<UniquePtr<Asset>> m_assets;
			Map<Str, Asset*> m_files;
			Map<Str, Plugin*> m_plugins;
	};

	class Asset{
		public:
			enum class Kind{
				file, mem, embed,

				count
			};

			using Access = resource::Access;

			enum class Category{
				binary, text, image, font, model, plugin,

				count
			};

			using DataPtrVar = std::variant<const void*, const char*>;

			virtual ~Asset() = default;

			Kind kind() const noexcept{ return m_kind; }
			Access access() const noexcept{ return m_access; }
			Category category() const noexcept{ return m_cat; }
			const Str &path() const noexcept{ return m_path; }

			bool hasRead(){ return (std::uint8_t)access() & (std::uint8_t)Access::read; }
			bool hasWrite(){ return (std::uint8_t)access() & (std::uint8_t)Access::write; }

			std::size_t size() const noexcept{ return m_size; }

			const void *data() const noexcept{ return m_data.r; } // this is basically always okay

			void *data() noexcept{
				if(hasWrite()){
					return m_data.w;
				}
				else{
					return nullptr;
				}
			}

		protected:
			Asset(Kind kind_, Access access_, Category cat_, Str path_) noexcept
				: m_kind(kind_), m_access(access_), m_path(std::move(path_)){}

			void setAccess(Access access_) noexcept{
				m_access = access_;
			}

			union DataPtr{
				const void *r;
				void *w;
			};

			void setData(Kind kind_, Access access_, std::size_t size_, DataPtr data_){
				m_kind = kind_;
				m_access = access_;
				m_size = size_;
				m_data = data_;
			}

			virtual void update(){}

		private:
			Kind m_kind;
			Access m_access;
			Category m_cat;
			Str m_path;
			std::size_t m_size = 0;
			DataPtr m_data;

			friend class Manager;
	};

	class Plugin: public Asset{
		public:
			using Proc = void(*)();

			virtual StrView name() const noexcept = 0;
			virtual StrView author() const noexcept = 0;
			virtual Version version() const noexcept = 0;

			virtual ManagerKind managerKind() const noexcept = 0;

			virtual UniquePtr<ManagerBase> createManager() const noexcept = 0;

			virtual Proc loadFunction(const Str &name) const noexcept = 0;

		protected:
			Plugin(Kind kind_, Str path_)
				: Asset(kind_, Access::read, Category::plugin, std::move(path_)){}
	};

	class Image: public Asset{
		public:
			Texture::Kind textureKind() const noexcept{ return m_texKind; }
			std::uint16_t width() const noexcept{ return m_w; }
			std::uint16_t height() const noexcept{ return m_h; }

		protected:
			Image(
				Kind kind_, Access access_, Str path_,
				Texture::Kind texKind_,
				std::uint16_t w_, std::uint16_t h_
			)
				: Asset(kind_, access_, Category::image, std::move(path_))
				, m_texKind(texKind_)
				, m_w(w_), m_h(h_)
			{}

			void setTextureData(Texture::Kind texKind_, std::uint16_t w_, std::uint16_t h_){
				m_texKind = texKind_;
				m_w = w_;
				m_h = h_;
			}

		private:
			Texture::Kind m_texKind;
			std::uint16_t m_w, m_h;
	};

	class Font: public Asset{
		public:
			class Face{
				public:
					virtual ~Face() = default;

					virtual void setPtSize(std::uint16_t pt) = 0;

					virtual StrView family() const noexcept = 0;
					virtual StrView style() const noexcept = 0;
			};

			virtual std::size_t numFaces() const noexcept = 0;
			virtual Face *face(std::size_t idx) noexcept = 0;
			virtual const Face *face(std::size_t idx) const noexcept = 0;

		protected:
			Font(
				Kind kind_, Access access_, Str path_
			)
				: Asset(kind_, access_, Category::font, std::move(path_)){}
	};

	class Model: public Asset{
		public:
			const Vector<shapes::TriangleMesh> &meshes() const noexcept{
				return m_meshes;
			}

		protected:
			Model(
				Kind kind_, Access access_, Str path_,
				Vector<shapes::TriangleMesh> meshes_
			)
				: Asset(kind_, access_, Category::model, std::move(path_))
				, m_meshes(std::move(meshes_))
			{
				setData(
					kind_, access_,
					m_meshes.size() * sizeof(shapes::TriangleMesh),
					{ .w = m_meshes.data() }
				);
			}

		private:
			Vector<shapes::TriangleMesh> m_meshes;
	};
}

#endif // !GPWE_RESOURCE_HPP

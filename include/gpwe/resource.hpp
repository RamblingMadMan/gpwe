#ifndef GPWE_RESOURCE_HPP
#define GPWE_RESOURCE_HPP 1

#include <memory>
#include <filesystem>

#include "String.hpp"
#include "Map.hpp"
#include "Shape.hpp"
#include "Texture.hpp"

namespace gpwe{
	namespace fs = std::filesystem;

	using Path = fs::path;
}

namespace gpwe::resource{
	class Asset{
		public:
			enum class Kind{
				file, mem,

				count
			};

			enum class Access: std::uint8_t{
				read = 0x1, write = 0x2, readWrite = read | write
			};

			enum class Category{
				binary, text, image, model,

				count
			};

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

	class Manager{
		public:
			Manager();
			~Manager();

			bool mount(const Path &path, StrView dir, bool mountBefore = true);

			Asset *openFile(StrView path, Asset::Access access_ = Asset::Access::read);
			Model *openModel(StrView path, Asset::Access access_ = Asset::Access::read);

			void update();

		private:
			UniquePtr<Model> createModelFileAsset(
				Str path, Asset::Access access_,
				Vector<char> bytes
			);

			Map<Str, Str> m_mounts;
			Vector<UniquePtr<Asset>> m_assets;
			Map<Str, Asset*> m_files;
	};
}

#endif // !GPWE_RESOURCE_HPP

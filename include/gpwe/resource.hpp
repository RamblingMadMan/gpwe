#ifndef GPWE_RESOURCE_HPP
#define GPWE_RESOURCE_HPP 1

#include <string_view>
#include <memory>
#include <vector>
#include <map>
#include <filesystem>

#include "Texture.hpp"

namespace gpwe{
	namespace fs = std::filesystem;
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
				image, text, binary,

				count
			};

			virtual ~Asset() = default;

			Kind kind() const noexcept{ return m_kind; }
			Access access() const noexcept{ return m_access; }
			Category category() const noexcept{ return m_cat; }
			std::string_view path() const noexcept{ return m_path; }

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
			Asset(Kind kind_, Access access_, Category cat_, std::string path_) noexcept
				: m_kind(kind_), m_access(access_), m_path(std::move(path_)){}

			void setDataRW(Kind kind_, std::size_t size_, void *data_){
				m_kind = kind_;
				m_access = Access::readWrite;
				m_size = size_;
				m_data.w = data_;
			}

			void setDataR(Kind kind_, std::size_t size_, const void *data_){
				m_kind = kind_;
				m_access = Access::read;
				m_size = size_;
				m_data.r = data_;
			}

		private:
			Kind m_kind;
			Access m_access;
			Category m_cat;
			std::string m_path;
			std::size_t m_size = 0;

			union {
				const void *r;
				void *w;
			} m_data;

			friend class Manager;
	};

	class Image: public Asset{
		public:
			Texture::Kind textureKind() const noexcept{ return m_texKind; }
			std::uint16_t width() const noexcept{ return m_w; }
			std::uint16_t height() const noexcept{ return m_h; }

		protected:
			Image(
				Kind kind_, Access access_, std::string path_,
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

			friend class Manager;
	};

	class Manager{
		public:
			Manager();
			~Manager();

			bool mount(const fs::path &path, std::string_view dir, bool mountBefore = true);

			Asset *openFile(std::string_view path);

		private:
			std::map<std::string, std::string> m_mounts;
			std::vector<std::unique_ptr<Asset>> m_assets;
			std::map<Asset*, void*> m_memMap;
	};
}

#endif // !GPWE_RESOURCE_HPP

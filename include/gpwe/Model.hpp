#ifndef GPWE_MODEL_HPP
#define GPWE_MODEL_HPP 1

#include <filesystem>

#include "Shape.hpp"

namespace gpwe{
	namespace fs = std::filesystem;

	class Model{
		public:
			explicit Model(const fs::path &filePath);

			const Shape *shape(std::uint32_t idx) const noexcept{
				if(idx >= m_meshes.size()){
					return nullptr;
				}

				return &m_meshes[idx];
			}

			std::vector<const Shape*> shapes() const;

		private:
			std::vector<shapes::TriangleMesh> m_meshes;
	};
}

#endif // !GPWE_MODEL_HPP

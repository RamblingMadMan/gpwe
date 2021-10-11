#ifndef GPWE_MODEL_HPP
#define GPWE_MODEL_HPP 1

#include <filesystem>

#include "Vector.hpp"
#include "Shape.hpp"

namespace gpwe{
	namespace fs = std::filesystem;

	class Model{
		public:
			Model(){}

			explicit Model(const fs::path &filePath);

			const VertexShape *shape(std::uint32_t idx) const noexcept{
				if(idx >= m_meshes.size()){
					return nullptr;
				}

				return &m_meshes[idx];
			}

			Vector<const VertexShape*> shapes() const;

			void loadModel(const fs::path &filePath);

		private:
			Vector<shapes::TriangleMesh> m_meshes;
	};
}

#endif // !GPWE_MODEL_HPP

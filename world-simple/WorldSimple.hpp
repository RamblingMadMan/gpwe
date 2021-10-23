#ifndef GPWE_WORLD_OCTREE_HPP
#define GPWE_WORLD_OCTREE_HPP 1

#include "gpwe/util/Octree.hpp"
#include "gpwe/world.hpp"

namespace worldSimple{
	using namespace gpwe;

	class Manager: public world::Manager{
		public:
			Manager();
			~Manager();

			void init() override;

		protected:
			UniquePtr<world::Block> doCreateBlock() override;
	};

	class Block: public world::Block{
		public:
			Block();
			~Block();

		protected:
			UniquePtr<world::Entity> doCreateEntity() override;
	};

	class Entity: public world::Entity{
		public:
			Entity();
			~Entity();
	};
}

#endif // !GPWE_WORLD_OCTREE_HPP

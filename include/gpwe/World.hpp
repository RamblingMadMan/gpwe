#ifndef GPWE_WORLD_HPP
#define GPWE_WORLD_HPP 1

#include "util/math.hpp"
#include "util/Octree.hpp"
#include "Manager.hpp"

namespace gpwe::world{
	class Block;
	class Entity;

	class Manager:
		public gpwe::Manager<
			Manager, ManagerKind::world,
			Block
		>
	{
		public:
			virtual ~Manager() = default;

		protected:
			virtual UniquePtr<Entity> doCreateBlock() = 0;
			friend class Block;
	};

	class Block:
		public gpwe::Managed<"WorldBlock"_cs, &Manager::doCreateBlock>,
		public gpwe::Manager<Block, ManagerKind::data, Entity>
	{
		public:
			virtual ~Block() = default;

		protected:
			virtual UniquePtr<Entity> doCreateEntity() = 0;
			friend class Entity;
	};

	class Entity: public gpwe::Managed<
		"world::Entity"_cs, &Block::doCreateEntity,
		Property<"position"_cs, Vec3>
	>{
		public:
			virtual ~Entity() = default;

			const Vec3 &rotation() const noexcept{ return m_rot; }

		private:
			Vec3 m_rot;
	};
}

#endif // !GPWE_WORLD_HPP

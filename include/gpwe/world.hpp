#ifndef GPWE_WORLD_HPP
#define GPWE_WORLD_HPP 1

#include "util/math.hpp"
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
			virtual UniquePtr<Block> doCreateBlock() = 0;
			friend class Block;
	};

	class Block:
		public gpwe::Managed<Block, &Manager::doCreateBlock>,
		public gpwe::Manager<Block, ManagerKind::data, Entity>
	{
		public:
			virtual ~Block() = default;

			virtual Vector<Entity*> traceLine(
				const Vec3 &from, const Vec3 &to, Nat8 maxHits = 1,
				const Vector<Entity*> &ignoredEntities = {},
				const Vector<ObjectClassBase*> &ignoredClasses = {}
			){
				return {};
			}

		protected:
			virtual UniquePtr<Entity> doCreateEntity() = 0;
			friend class Entity;
	};

	class Entity:
		public gpwe::Managed<
			Entity, &Block::doCreateEntity,
			Property<"name"_cs, Str>,
			Property<"position"_cs, Vec3>,
			Property<"rotation"_cs, Vec3>,
			Property<"scale"_cs, Vec3>,
			Property<"aabb"_cs, AABB>
		>
	{
		public:
			explicit Entity(Entity *parent_ = nullptr);

			virtual ~Entity() = default;
	};
}

#define GPWE_WORLD_PLUGIN(type, name, author, major, minor, patch)\
	GPWE_PLUGIN(world, type, name, author, major, minor, patch)

#endif // !GPWE_WORLD_HPP

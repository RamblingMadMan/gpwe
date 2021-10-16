#ifndef GPWE_PHYSICS_HPP
#define GPWE_PHYSICS_HPP 1

#include "Manager.hpp"
#include "Shape.hpp"

namespace gpwe::physics{
	class BodyShape;
	class Body;
	class World;

	class Manager: public gpwe::Manager<Manager, ManagerKind::physics, World>{
		public:
			virtual ~Manager() = default;

			void update(float dt);

		protected:
			virtual UniquePtr<World> doCreateWorld() = 0;

			friend class World;
	};

	class World :
		public Managed<Manager, &Manager::doCreateWorld>,
		public gpwe::Manager<World, ManagerKind::data, BodyShape, Body>
	{
		public:
			virtual ~World() = default;

			virtual void update(float dt) = 0;

		protected:
			virtual UniquePtr<BodyShape> doCreateBodyShape(const Shape *shape) = 0;
			virtual UniquePtr<Body> doCreateBody(const BodyShape *bodyShape) = 0;

		private:
			friend class BodyShape;
			friend class Body;
	};

	class BodyShape: public Managed<World, &World::doCreateBodyShape>{
		public:
			virtual ~BodyShape() = default;
	};

	class Body: public Managed<World, &World::doCreateBody>{
		public:
			virtual ~Body() = default;

			virtual float mass() const noexcept = 0;
			virtual glm::vec3 position() const noexcept = 0;
	};
}

#define GPWE_PHYSICS(type, name, author, major, minor, patch)\
	GPWE_PLUGIN(physics, type, name, author, major, minor, patch)

#endif // !GPWE_PHYSICS_HPP

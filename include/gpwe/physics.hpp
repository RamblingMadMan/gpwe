#ifndef GPWE_PHYSICS_HPP
#define GPWE_PHYSICS_HPP 1

#include "List.hpp"
#include "Shape.hpp"

namespace gpwe::physics{
	class BodyShape;
	class Body;
	class World;

	class Manager{
		public:
			virtual ~Manager() = default;

			void update(float dt);

			World *createWorld();
			bool destroyWorld(World *world);

		protected:
			virtual UniquePtr<World> doCreateWorld() = 0;

			gpwe::List<UniquePtr<BodyShape>> m_shapes;
			gpwe::List<UniquePtr<World>> m_worlds;
	};

	class BodyShape{
		public:
			virtual ~BodyShape() = default;
	};

	class Body{
		public:
			virtual ~Body() = default;

			virtual float mass() const noexcept = 0;
			virtual glm::vec3 position() const noexcept = 0;
	};

	class World{
		public:
			virtual ~World() = default;

			virtual void update(float dt) = 0;

			BodyShape *createBodyShape(const Shape *shape);
			bool destroyBodyShape(BodyShape *bodyShape);

			Body *createBody(const BodyShape *bodyShape);
			bool destroyBody(Body *body);

		protected:
			virtual UniquePtr<BodyShape> doCreateBodyShape(const Shape *shape) = 0;
			virtual UniquePtr<Body> doCreateBody(const BodyShape *bodyShape) = 0;

		private:
			gpwe::List<UniquePtr<BodyShape>> m_shapes;
			gpwe::List<UniquePtr<Body>> m_bodies;
	};
}

#endif // !GPWE_PHYSICS_HPP

#ifndef GPWE_PHYSICS_BULLET3_HPP
#define GPWE_PHYSICS_BULLET3_HPP

#include "gpwe/physics.hpp"

class btCollisionConfiguration;
class btDispatcher;
class btCollisionShape;
class btConstraintSolver;
class btSoftRigidDynamicsWorld;
class btBroadphaseInterface;
class btRigidBody;

namespace gpwe::physics::bullet3{
	class Manager: public physics::Manager{
		public:
			Manager();
			~Manager();

		protected:
			UniquePtr<physics::World> doCreateWorld() override;

		private:
			UniquePtr<btCollisionConfiguration> m_config;
			UniquePtr<btDispatcher> m_dispatcher;
			UniquePtr<btConstraintSolver> m_solver;
	};

	class BodyShape: public physics::BodyShape{
		public:
			explicit BodyShape(const gpwe::VertexShape *shape);
			explicit BodyShape(const gpwe::HeightMapShape *shape);
			~BodyShape();

			btCollisionShape *collisionShape() const noexcept{ return m_shape.get(); }

		private:
			mutable UniquePtr<btCollisionShape> m_shape;
	};

	class RigidBody: public physics::Body{
		public:
			RigidBody(
				btSoftRigidDynamicsWorld *world, const BodyShape *bodyShape,
				float mass
			);

			~RigidBody();

			float mass() const noexcept override{ return m_mass; }
			glm::vec3 position() const noexcept override{ return m_pos; }

		private:
			btSoftRigidDynamicsWorld *m_world;
			float m_mass;
			glm::vec3 m_pos;
			UniquePtr<btRigidBody> m_body;
	};

	class World: public physics::World{
		public:
			World(btCollisionConfiguration *config, btDispatcher *dispatcher, btConstraintSolver *solver);
			~World();

			void update(float dt) override;

		protected:
			UniquePtr<physics::BodyShape> doCreateBodyShape(const gpwe::Shape *shape) override;
			UniquePtr<physics::Body> doCreateBody(const physics::BodyShape *bodyShape, float mass) override;

		private:
			UniquePtr<btBroadphaseInterface> m_broadphase;
			UniquePtr<btSoftRigidDynamicsWorld> m_world;
	};
}

#endif // GPWE_PHYSICS_BULLET3_HPP

#ifndef GPWE_PHYSICS_BULLET3_HPP
#define GPWE_PHYSICS_BULLET3_HPP

#include "gpwe/physics.hpp"

class b3CpuNarrowPhase;
class b3CpuRigidBodyPipeline;
class b3DynamicBvhBroadphase;

namespace gpwe::physics::bullet3{
	class Manager: public physics::Manager{
		public:

		protected:
			UniquePtr<physics::World> doCreateWorld() override;			
	};

	class BodyShape: public physics::BodyShape{
		public:
			BodyShape(b3CpuRigidBodyPipeline *pipeline, const gpwe::VertexShape *shape);
			~BodyShape();

			const b3CpuRigidBodyPipeline *pipeline() const noexcept{ return m_pipeline; }
			int pipelineIndex() const noexcept{ return m_idx; }

		private:
			b3CpuRigidBodyPipeline *m_pipeline;
			int m_idx;
	};

	class Body: public physics::Body{
		public:
			Body(b3CpuRigidBodyPipeline *pipeline, const BodyShape *bodyShape);
			~Body();

			float mass() const noexcept override{ return m_mass; }
			glm::vec3 position() const noexcept override{ return m_pos; }

			const b3CpuRigidBodyPipeline *pipeline() const noexcept{ return m_pipeline; }
			int pipelineIndex() const noexcept{ return m_idx; }

		private:
			b3CpuRigidBodyPipeline *m_pipeline;
			float m_mass;
			glm::vec3 m_pos;
			int m_idx;
	};

	class World: public physics::World{
		public:
			World();
			~World();

			void update(float dt) override;

		protected:
			UniquePtr<physics::BodyShape> doCreateBodyShape(const gpwe::Shape *shape) override;
			UniquePtr<physics::Body> doCreateBody(const physics::BodyShape *bodyShape) override;

		private:
			UniquePtr<b3DynamicBvhBroadphase> m_broadPhase;
			UniquePtr<b3CpuNarrowPhase> m_narrowPhase;
			UniquePtr<b3CpuRigidBodyPipeline> m_pipeline;
	};
}

#endif // GPWE_PHYSICS_BULLET3_HPP

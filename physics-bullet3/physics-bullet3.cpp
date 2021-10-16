#include "gpwe/log.hpp"

#include "glm/gtc/type_ptr.hpp"

#include "Bullet3Dynamics/b3CpuRigidBodyPipeline.h"
#include "Bullet3Collision/NarrowPhaseCollision/b3ConvexUtility.h"
#include "Bullet3Collision/NarrowPhaseCollision/b3Config.h"
#include "Bullet3Collision/BroadPhaseCollision/b3DynamicBvhBroadphase.h"
#include "Bullet3Collision/NarrowPhaseCollision/b3CpuNarrowPhase.h"
#include "Bullet3Geometry/b3ConvexHullComputer.h"

#include "physics-bullet3.hpp"

using namespace gpwe;
using namespace gpwe::physics::bullet3;

UniquePtr<physics::World> Manager::doCreateWorld(){
	return makeUnique<World>();
}

BodyShape::BodyShape(b3CpuRigidBodyPipeline *pipeline, const gpwe::VertexShape *shape)
	: m_pipeline(pipeline)
{
	b3ConvexHullComputer computer;

	computer.compute((const float*)shape->vertices(), sizeof(glm::vec3), shape->numPoints(), 0, 0);

	b3ConvexUtility convex;
	convex.initialize();
	convex.initializePolyhedralFeatures(&computer.vertices[0], computer.vertices.size());

	m_idx = pipeline->registerConvexPolyhedron(&convex);
}

BodyShape::~BodyShape(){
	m_pipeline->removeConstraintByUid(m_idx);
}

Body::Body(b3CpuRigidBodyPipeline *pipeline, const BodyShape *bodyShape)
	: m_pipeline(pipeline)
	, m_mass(1.f)
	, m_pos(0.f, 0.f, 0.f)
{
	glm::vec3 orientation{0.f, 0.f, 0.f};
	m_idx = pipeline->registerPhysicsInstance(
		m_mass, glm::value_ptr(m_pos), glm::value_ptr(orientation),
		bodyShape->pipelineIndex(), 0
	);
}

Body::~Body(){
	m_pipeline->removeConstraintByUid(m_idx);
}

World::World()
	: m_broadPhase(makeUnique<b3DynamicBvhBroadphase>(64))
	, m_narrowPhase(makeUnique<b3CpuNarrowPhase>(b3Config{}))
	, m_pipeline(makeUnique<b3CpuRigidBodyPipeline>(m_narrowPhase.get(), m_broadPhase.get(), b3Config{}))
{
}

World::~World(){
	// make sure pipeline is destroyed first
	m_pipeline.reset();
}

void World::update(float dt){
	m_pipeline->stepSimulation(dt);
}

UniquePtr<physics::BodyShape> World::doCreateBodyShape(const gpwe::Shape *shape){
	auto mesh = dynamic_cast<const gpwe::VertexShape*>(shape);
	if(!mesh){
		logErrorLn("Bullet3 Physics Error: Only gpwe::VertexShape currently supported");
		return nullptr;
	}

	return makeUnique<BodyShape>(m_pipeline.get(), mesh);
}

UniquePtr<physics::Body> World::doCreateBody(const physics::BodyShape *bodyShape){
	auto derived = dynamic_cast<const BodyShape*>(bodyShape);
	if(!derived){
		return nullptr;
	}

	return makeUnique<Body>(m_pipeline.get(), derived);
}

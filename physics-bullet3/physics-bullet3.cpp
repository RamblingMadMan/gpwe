#include "gpwe/util/WorkQueue.hpp"
#include "gpwe/util/Thread.hpp"
#include "gpwe/log.hpp"

#include "glm/gtc/type_ptr.hpp"

#include "LinearMath/btThreads.h"

#include "Bullet3Geometry/b3ConvexHullComputer.h"

#include "BulletCollision/CollisionShapes/btBoxShape.h"
#include "BulletCollision/CollisionShapes/btCompoundShape.h"
#include "BulletCollision/CollisionShapes/btConvexHullShape.h"
#include "BulletCollision/CollisionShapes/btConvexPolyhedron.h"
#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"

#include "BulletCollision/CollisionDispatch/btCollisionDispatcherMt.h"

#include "BulletCollision/BroadphaseCollision/btDbvtBroadphase.h"

#include "BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolverMt.h"

#include "BulletDynamics/Dynamics/btRigidBody.h"

#include "BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h"
#include "BulletSoftBody/btSoftRigidDynamicsWorld.h"

#include "physics-bullet3.hpp"

GPWE_PHYSICS_PLUGIN(gpwe::physics::bullet3::Manager, "Bullet Physics Plugin", "Hamsmith", 0, 0, 0)

using namespace gpwe;
using namespace gpwe::physics::bullet3;

physics::bullet3::Manager::Manager(){
	// TODO: inspect this monstrosity
	btDefaultCollisionConstructionInfo info;

	m_config = makeUnique<btSoftBodyRigidBodyCollisionConfiguration>(info);
	m_dispatcher = makeUnique<btCollisionDispatcher>(m_config.get());
	m_solver = makeUnique<btSequentialImpulseConstraintSolverMt>();

	btITaskScheduler *scheduler = btGetTBBTaskScheduler();
	if(!scheduler){
		scheduler = btGetOpenMPTaskScheduler();
		if(!scheduler){
			#ifdef _WIN32
			scheduler = btGetPPLTaskScheduler();
			if(!scheduler){
			#endif

				scheduler = btGetSequentialTaskScheduler();

			#ifdef _WIN32
			}
			#endif
		}
	}

	btSetTaskScheduler(scheduler);
}

physics::bullet3::Manager::~Manager(){

}

UniquePtr<physics::World> physics::bullet3::Manager::doCreateWorld(){
	return makeUnique<World>(m_config.get(), m_dispatcher.get(), m_solver.get());
}

BodyShape::BodyShape(const gpwe::VertexShape *shape)
{
	if(!shape){
		log::errorLn("NULL passed as VertexShape to physics::BodyShape");
		// TODO: throw exception?
		return;
	}

	b3ConvexHullComputer computer;

	Vector<b3Vector3> btVecs;
	btVecs.reserve(shape->numPoints());

	std::transform(
		shape->vertices(), shape->vertices() + shape->numPoints(),
		std::back_inserter(btVecs),
		[](const glm::vec3 &v){ return b3Vector3{v.x, v.y, v.z}; }
	);

	computer.compute((const float*)btVecs.data(), sizeof(btVector3), btVecs.size(), 0, 0);

	m_shape = makeUnique<btConvexHullShape>(&computer.vertices[0][0], computer.vertices.size());
}

BodyShape::BodyShape(const gpwe::HeightMapShape *shape)
{
	if(!shape){
		log::errorLn("NULL passed as HeightMapShape to physics::BodyShape");
		return;
	}

	m_shape = makeUnique<btHeightfieldTerrainShape>(shape->width(), shape->height(), shape->values(), 0.f, 1.f, 1, false);

	btVector3 aabbMin;
	btVector3 aabbMax;
	m_shape->getAabb(btTransform{}, aabbMin, aabbMax);

	property<"aabb"_cs>() = AABB{
		glm::make_vec3(&aabbMin[0]),
		glm::make_vec3(&aabbMax[0])
	};
}

BodyShape::~BodyShape(){}

RigidBody::RigidBody(btSoftRigidDynamicsWorld *world, const BodyShape *bodyShape, float mass_)
	: m_world(world)
	, m_mass(mass_)
	, m_pos(0.f, 0.f, 0.f)
{
	glm::vec3 orientation{0.f, 0.f, 0.f};

	btRigidBody::btRigidBodyConstructionInfo info(mass_, nullptr, bodyShape->collisionShape());

	m_body = makeUnique<btRigidBody>(info);

	world->addRigidBody(m_body.get());
}

RigidBody::~RigidBody(){
	m_world->removeRigidBody(m_body.get());
}

World::World(btCollisionConfiguration *config, btDispatcher *dispatcher, btConstraintSolver *solver){
	m_broadphase = makeUnique<btDbvtBroadphase>();
	m_world = makeUnique<btSoftRigidDynamicsWorld>(
		dispatcher,
		m_broadphase.get(),
		solver,
		config
	);
}

World::~World(){
}

void World::update(float dt){
	const float physicsDt = 1.f/120.f;
	m_world->stepSimulation(dt, 100, physicsDt);
}

UniquePtr<physics::BodyShape> World::doCreateBodyShape(const gpwe::Shape *shape){
	if(auto field = dynamic_cast<const gpwe::HeightMapShape*>(shape)){
		return makeUnique<BodyShape>(field);
	}
	else if(auto mesh = dynamic_cast<const gpwe::VertexShape*>(shape)){
		return makeUnique<BodyShape>(mesh);
	}
	else{
		log::errorLn("Bullet3 Physics Error: Only VertexShape and HeightMapShape currently supported");
		return nullptr;
	}
}

UniquePtr<physics::Body> World::doCreateBody(const physics::BodyShape *bodyShape, float mass){
	auto derived = dynamic_cast<const BodyShape*>(bodyShape);
	if(!derived){
		return nullptr;
	}

	return makeUnique<RigidBody>(m_world.get(), derived, mass);
}

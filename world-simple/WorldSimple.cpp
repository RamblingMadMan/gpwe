#include "WorldSimple.hpp"

using namespace gpwe;

GPWE_WORLD_PLUGIN(worldSimple::Manager, "Simple World", "RamblingMad", 0, 0, 0)

worldSimple::Manager::Manager(){}

worldSimple::Manager::~Manager(){}

void worldSimple::Manager::init(){}

UniquePtr<world::Block> worldSimple::Manager::doCreateBlock(){
	return makeUnique<worldSimple::Block>();
}

worldSimple::Block::Block(){}
worldSimple::Block::~Block(){}

UniquePtr<world::Entity> worldSimple::Block::doCreateEntity(){
	return makeUnique<worldSimple::Entity>();
}

worldSimple::Entity::Entity(){}

worldSimple::Entity::~Entity(){}

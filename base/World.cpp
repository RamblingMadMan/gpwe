#include "gpwe/world.hpp"

using namespace gpwe;
using namespace gpwe::world;

Entity::Entity(Entity *parent_)
	: Entity::ManagedType(parent_){}

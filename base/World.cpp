#include "gpwe/world.hpp"

using namespace gpwe;
using namespace gpwe::world;

Vector<Entity*> world::traceLine(const Vec3 &from, const Vec3 &to, Nat8 maxHits){
	Vector<Entity*> ret;
	ret.reserve(maxHits);

	// TODO: implement trace within the world

	return ret;
}

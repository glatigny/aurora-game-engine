#include "collision_point.h"
#define HEADER collision_point
#define HELPER scriptable_object_cpp
#include "include_helper.h"

CollisionPoint::CollisionPoint(State* attachedState, Translatable* offset, int layer, int collisionLayers) : Translatable(offset), Entity(attachedState), Collidable(attachedState, offset, NULL, layer, collisionLayers) {

}

CollisionPoint::~CollisionPoint() {

}

Collision CollisionPoint::hitBy(Collidable* other) {
	if(!(getCollisionLayers() & other->getCollisionLayers()))
		return Collision(this, NULL);

	switch( other->getCollisionType() )
	{
		case COLLISION_TYPE_POINT:
			return hitBy((CollisionPoint*)other);
	}

	// The other one is not a CollisionPoint.
	// We need to test it against a CollisionPoint.
	// This call gives us a reversed Collision, so let's swap it.
	Collision reversedCol = other->hitBy(this);
	reversedCol.swap();
	return reversedCol;
}

Collision CollisionPoint::hitBy(CollisionPoint* other) {
	if(!(getCollisionLayers() & other->getCollisionLayers()))
		return Collision(this, NULL);
			
	Collision ret(this, NULL);

	if( (other->getX() == this->getX()) && (other->getY() == this->getY()) )
	{
		ret.setPosition(this->getX(), this->getY());
		ret.setCollidedObject(other);
		ret.setCollidingHull(this);
		ret.setCollidedHull(other);
	}
	return ret;
}

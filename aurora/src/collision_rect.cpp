#include "collision_point.h"
#include "collision_rect.h"

#ifdef DISPLAY_COLLISIONS
#include "aurora_engine.h"
#endif

#define HEADER collision_rect
#define HELPER scriptable_object_cpp
#include "include_helper.h"

CollisionRect::CollisionRect(State* attachedState, Translatable* offset, int layer, int collisionLayers, Point newSize) : Translatable(offset), Entity(attachedState), Collidable(attachedState, offset, NULL, layer, collisionLayers), Resizable(newSize.getX(), newSize.getY()) {
}

CollisionRect::~CollisionRect() {

}

// QC:G
Collision CollisionRect::hitBy(Collidable* other) {
	if(!(getCollisionLayers() & other->getCollisionLayers()))
		return Collision(this, NULL);
		
	switch( other->getCollisionType() )
	{
		case COLLISION_TYPE_POINT:
			return hitBy((CollisionPoint*)other);
		case COLLISION_TYPE_RECT:
			return hitBy((CollisionRect*)other);
	}

	// The other one is not a known Collision.
	// This call gives us a reversed Collision, so let's swap it.
	Collision reversedCol = other->hitBy(this);
	reversedCol.swap();
	return reversedCol;
}

// QC:A
Collision CollisionRect::hitBy(CollisionRect* other) {
	if(!(getCollisionLayers() & other->getCollisionLayers()))
		return Collision(this, NULL);
		
	Collision ret(this, NULL);

	// Very simple version.
	// No rotation
	
	int x = getX();
	int y = getY();
	int w = size.getX();
	int h = size.getY();
	
	int ox = other->getX();
	int oy = other->getY();
	int ow = other->size.getX();
	int oh = other->size.getY();
	
	if( !(x > ox+ow) && !(ox > x+w) && !(y > oy+oh) && !(oy > y+h) ) {
#ifdef DISPLAY_COLLISIONS
		if(state()->displayCollisions) {
			engine()->hardware()->drawRect(&Rect(x, y, w, h), 255, 64, 64, 192);
			engine()->hardware()->drawRect(&Rect(ox, oy, ow, oh), 255, 64, 64, 192);
		}
#endif
		ret.setPosition(x, y); // Incorrect value
		ret.setCollidedObject(other);
		ret.setCollidingHull(this);
		ret.setCollidedHull(other);
	}
#ifdef DISPLAY_COLLISIONS
	else {
		if(state()->displayCollisions) {
			engine()->hardware()->drawRect(&Rect(x, y, w, h), 255, 64, 255, 192);
			engine()->hardware()->drawRect(&Rect(ox, oy, ow, oh), 255, 64, 255, 192);
		}
	}
#endif
	
	return ret;
}

// QC:G (FIXME : test is in object space)
Collision CollisionRect::hitBy(CollisionPoint* other) {
	if(!(getCollisionLayers() & other->getCollisionLayers()))
		return Collision(this, NULL);

	Collision ret(this, NULL);

	// Very simple version.
	// No rotation
	
	int ox = other->getX();
	int oy = other->getY();
	int x = getX();
	int y = getY();
	int w = size.getX();
	int h = size.getY();
	
	if( (ox >= x) && (ox <= x+w) && (oy >= y) && (oy <= y+h) ) {
#ifdef DISPLAY_COLLISIONS
		if(state()->displayCollisions) {
			engine()->hardware()->drawRect(&Rect(x, y, w, h), 255, 64, 64, 192);
			engine()->hardware()->drawRect(&Rect(ox, oy, 1, 1), 255, 192, 64, 224);
			engine()->hardware()->drawRect(&Rect(ox - 16, oy - 16, 33, 33), 255, 192, 64, 120);
		}
#endif
		ret.setPosition(x, y);
		ret.setCollidedObject(other);
		ret.setCollidingHull(this);
		ret.setCollidedHull(other);
		ret.enableLayers(getCollisionLayers() & other->getCollisionLayers());
	}
#ifdef DISPLAY_COLLISIONS
	else {
		if(state()->displayCollisions) {
			engine()->hardware()->drawRect(&Rect(x, y, w, h), 255, 64, 255, 192);
			engine()->hardware()->drawRect(&Rect(ox, oy, 1, 1), 255, 127, 64, 224);
			engine()->hardware()->drawRect(&Rect(ox - 16, oy - 16, 33, 33), 255, 127, 64, 120);
		}
	}
#endif
	
	return ret;
}

// QC:A (FIXME : returns dist in object space)
bool CollisionRect::getDistanceFromBorders(Collidable* c, Resizable* dist) {
	Point op = getRelativePosition(c);
	dist->setPosition(op);
	Point s = getSize();
	dist->setSize(-s);
	return op.getX() >= 0 && op.getX() <= size.getX();
}


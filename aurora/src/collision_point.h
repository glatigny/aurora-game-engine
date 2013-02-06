#ifndef COLLISION_POINT_H
#define COLLISION_POINT_H

#include "collidable.h"
#include "state.h"

/** A CollisionPoint is a Point which can collide with another collidables
 */
class CollisionPoint : public Collidable {
private:
	
protected:

public:
#define HEADER collision_point
#define HELPER scriptable_object_h
#include "include_helper.h"
	CollisionPoint() : Collidable() {}
	CollisionPoint(State* attachedState, Translatable* offset, int layer, int collisionLayers);
	virtual ~CollisionPoint();

	/* Collidable */
	
	virtual Translatable* getLeftmostPoint() {
		return (Translatable*)this;
	}
	
	virtual Translatable* getRightmostPoint() {
		return (Translatable*)this;
	}
	
	virtual inline int getCollisionType() { return COLLISION_TYPE_POINT; }

	virtual Point getSize() { return Point(0,0); }
	virtual Collision hitBy(Collidable* other);

	Collision hitBy(CollisionPoint* other);

	/* Renderable */
	
	/** CollisionHulls are not renderable, so this function does nothing. */
	void renderGraphics() { }

	/** CollisionHulls are static, so this function does nothing. */
	void updateObject(dur elapsedTime) { }

	virtual void loadGraphics() {}
	virtual void unloadGraphics() {}
};

#endif

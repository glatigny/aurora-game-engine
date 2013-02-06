#ifndef COLLISION_RECT_H
#define COLLISION_RECT_H

#include "collidable.h"
#include "state.h"

/** A CollisionPoint is a Point which can collide with another collidables
 */
class CollisionRect : public Collidable, public Resizable {

public:
#define HEADER collision_rect
#define HELPER scriptable_object_h
#include "include_helper.h"
	CollisionRect() : Collidable() {}
	CollisionRect(State* attachedState, Translatable* offset, int layer, int collisionLayers, Point size);
	virtual ~CollisionRect();

	/* Collidable */
	
	virtual Translatable* getLeftmostPoint() {
		return (Translatable*)this;
	}
	
	virtual Translatable* getRightmostPoint() {
		return (Translatable*)this;
	}
	
	virtual inline int getCollisionType() { return COLLISION_TYPE_RECT; }

	virtual Point getSize() { return size; }
	virtual Collision hitBy(Collidable* other);

	Collision hitBy(CollisionPoint* other);
	Collision hitBy(CollisionRect* other);

	virtual bool getDistanceFromBorders(Collidable* c, Resizable* dist);

	/* Renderable */
	
	/** CollisionHulls are not renderable, so this function does nothing. */
	void renderGraphics() { }

	/** CollisionHulls are static, so this function does nothing. */
	void updateObject(dur elapsedTime) { }

	virtual void loadGraphics() {}
	virtual void unloadGraphics() {}
};

#endif

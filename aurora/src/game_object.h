#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include <map>

#include "collidable.h"
#include "state.h"

/** A game object with a fixed base point and fixed collision hull.
 */
class GameObject : public Collidable {
private:
	Collidable* collisionHull;
public:
#define HEADER game_object
#define HELPER scriptable_object_h
#include "include_helper.h"
	GameObject() : collisionHull(NULL) {}
	GameObject(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, int collisionLayers, Collidable* collision);
	virtual ~GameObject() {}

	/** Return the current collision hull for this object.
	 */
	virtual Collidable* getCollisionHull() {
		return collisionHull;
	}

	/** Set the collision hull of this object.
	 * Collision hull is the zone that actually matters for collisions.
	 * Any collidable can be set, so avoid loops. It is strongly advised
	 * to use an instance of CollisionHull if no dynamic collisions are needed.
	 * @param newCollisionHull a Collidable which will be used for collisions with this object.
	 * @see Collidable, CollisionHull
	 */
	void setCollisionHull(Collidable* newCollisionHull) {
		collisionHull = newCollisionHull;
		if(collisionHull)
			collisionHull->setParent(this);
	}

	/* Collidable */

	virtual Translatable* getLeftmostPoint();
	virtual Translatable* getRightmostPoint();

	virtual Point getSize();
	virtual Collision hitBy(Collidable* other);

	virtual bool getDistanceFromBorders(Collidable* c, Resizable* dist);
	
	/* Signalable */
	virtual int signal(ScriptableObject* other, SignalId signalId, int parameter) { if(collisionHull) return collisionHull->signal(other, signalId, parameter); else return 0; }

};

#endif


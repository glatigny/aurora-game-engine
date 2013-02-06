#ifndef COLLISION_H
#define COLLISION_H

class Collidable;
#include "translatable.h"

/** A collision. When two objects collide, they produce a collision.
 * A collision has an impact point (given by its coordinates)
 * and two Collidables (collidingObject and collidedObject).
 */
class Collision : public Translatable {
private:
	Collidable* collidingObject; /** The object which hit the other object. It is related to the member of the hitBy function. */
	Collidable* collidedObject; /** The object which has been hit by the member. It is related to the "other" parameter of the hitBy function. */
	
	Collidable* collidingHull; // The collision hull of the collidingObject which triggered collision.
	Collidable* collidedHull; // The collision hull of the collidedObject which triggered collision.
	
	int layers;
	coord angle; // Collision angle

public:
	Collision() {
		collidingObject = NULL;
		collidedObject = NULL;
		layers = 0;
	}
	
	Collision(Collidable* p_collidingObject, Collidable* p_collidedObject) {
		collidingObject = p_collidingObject;
		collidedObject = p_collidedObject;
		layers = 0;
	}

	/** Returns the colliding object.
	 * The colliding object is the object which hit the other object.
	 * It is related to the member of the hitBy function.
	 * @return the colliding object.
	 */
	Collidable* getCollidingObject() {
		return collidingObject;
	}

	/** Returns the collided object.
	 * The collided object is the object which has been hit by the member.
	 * It is related to the "other" parameter of the hitBy function.
	 * @return the collided object.
	 */
	Collidable* getCollidedObject() {
		return collidedObject;
	}

	/** Returns the colliding hull.
	 * The colliding object is the object which hit the other object.
	 * This points to the object that triggered the collision.
	 * @return the colliding object hull.
	 */
	Collidable* getCollidingHull() {
		return collidingHull;
	}

	/** Returns the collided hull.
	 * The collided object is the object which hit the other object.
	 * This points to the object that triggered the collision.
	 * @return the collided object hull.
	 */
	Collidable* getCollidedHull() {
		return collidedHull;
	}
	
	void setCollidingHull(Collidable* newCollidingHull) {
		collidingHull = newCollidingHull;
	}

	void setCollidedHull(Collidable* newCollidedHull) {
		collidedHull = newCollidedHull;
	}

	/** Sets the colliding object.
	 * The collided object is the object which hit the other object.
	 * It is related to the member of the hitBy function.
	 * @param realCollidingObject the collidingObject really responsible for collision.
	 */
	void setCollidingObject(Collidable* realCollidingObject) {
		collidingObject = realCollidingObject;
	}

	/** Sets the collided object.
	 * The collided object is the object which has been hit by the member.
	 * It is related to the "other" parameter of the hitBy function.
	 * @param realCollidedObject the collidedObject really responsible for collision.
	 */
	void setCollidedObject(Collidable* realCollidedObject) {
		collidedObject = realCollidedObject;
	}
	
	/** Add new enabled layers.
	 * @param newLayers bitmask of the layers to enable.
	 */
	void enableLayers(int newLayers) {
		layers |= newLayers;
	}
	
	void filterLayers(int parentLayers) {
		layers &= parentLayers;
	}
	
	int getCollidedLayers() {
		return layers;
	}
	
	int getAngle() {
		return angle;
	}
	
	void setAngle(int newAngle) {
		angle = newAngle;
	}

	/** Test if a collision happened.
	 * @return true if a collision happened, false otherwise.
	 */
	bool collided() {
		return collidingObject && collidedObject;
	}

	/** Swap colliding object and collided object.
	 * For use by collision_hull.cpp
	 */
	void swap() {
		Collidable* tmp = collidingObject;
		collidingObject = collidedObject;
		collidedObject = tmp;
		
		tmp = collidingHull;
		collidingHull = collidedHull;
		collidedHull = tmp;
	}

	/* Translatable */

	/** Tests if the collision point is inside a rectangle zone.
	 */
	virtual bool inside(Translatable& firstCorner, Translatable& secondCorner) {
		return (
				(getX() <= firstCorner.getX() && getX() >= secondCorner.getX())
				||
				(getX() >= firstCorner.getX() && getX() <= secondCorner.getX())
			) && (
				(getY() <= firstCorner.getY() && getY() >= secondCorner.getY())
				||
				(getY() >= firstCorner.getY() && getY() <= secondCorner.getY())
			);
	}
};

#endif


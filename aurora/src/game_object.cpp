#include "game_object.h"

#define HEADER game_object
#define HELPER scriptable_object_cpp
#include "include_helper.h"

GameObject::GameObject(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, int collisionLayers, Collidable* collision) : Translatable(offset), Collidable(attachedState, offset, effects, layer, collisionLayers) {
	collisionHull = NULL;
	setCollisionHull(collision);
}

// QC:P
Collision GameObject::hitBy(Collidable* other) {
	Collidable* col = getCollisionHull();
	if(!collisionsEnabled || other == NULL || col == NULL || other == this) {
		return Collision();
	}
	
	// Translate the collisionHull to other's coordinate space
	Point chp(*col); // Save actual position
	col->translate(*getAbsolutePosition());
	
	// Test collision between the two objects
	Collision c;
	c = col->hitBy(other);

	// Put the collisionHull back where we found it
	col->setPosition(chp);

	// If there was a collision, the colliding object is this object,
	// and not its collisionHull. Same remark for collidedObject.
	if(c.collided()) {
		c.setCollidingObject(this);
		c.setCollidedObject(other);
		c.translate(-getAbsolutePosition()->getX(), -getAbsolutePosition()->getY());
	}
	
	c.filterLayers(getCollisionLayers());
	
	return c;
}

// QC:A
Translatable* GameObject::getLeftmostPoint() {
	static Point absoluteLeftmostPoint;
	if(collisionHull) {
		absoluteLeftmostPoint = *collisionHull->getLeftmostPoint();
		absoluteLeftmostPoint.translate(*collisionHull); // XXX Take scale in account
	} else {
		absoluteLeftmostPoint.setPosition(0, 0);
	}
	absoluteLeftmostPoint.translate(*this);
	return &absoluteLeftmostPoint;
}

// QC:A
Translatable* GameObject::getRightmostPoint() {
	static Point absoluteRightmostPoint;
	if(collisionHull) {
		absoluteRightmostPoint = *collisionHull->getRightmostPoint();
		absoluteRightmostPoint.translate(*collisionHull);
	} else {
		absoluteRightmostPoint.setPosition(0, 0);
	}
	absoluteRightmostPoint.translate(*this);
	return &absoluteRightmostPoint;
}

// QC:P
Point GameObject::getSize() {
	return collisionHull->getSize();
}

// QC:?
bool GameObject::getDistanceFromBorders(Collidable* c, Resizable* dist) {
	return collisionHull->getDistanceFromBorders(c, dist);
}


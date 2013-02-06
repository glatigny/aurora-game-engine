#include <iostream>

#include "collidable.h"

#define HEADER collidable
#define HELPER scriptable_object_cpp
#include "include_helper.h"

int Collidable::signal(ScriptableObject* other, SignalId signalId, int data) {
	Collidable* otherCollidable = dynamic_cast<Collidable*>(other);
	if(otherCollidable) {
		/*XXX*/
		//return signal(otherCollidable, signalId, data);
	}

	return 0;
}

void Collidable::setCollisionLayers(int newLayers) {
	collisionLayers = newLayers;
}

void Collidable::setCollisionLayer(int layer, bool newLayerValue) {
	xassert(layer >= 0 && layer <= 32, "Invalid collision layer : %d", layer);
	if(newLayerValue) {
		collisionLayers |= (1 << layer);
	} else {
		collisionLayers &=~ (1 << layer);
	}
}

// QC:P
bool Collidable::inside(Translatable& firstCorner, Translatable& secondCorner) {
	int minX = (firstCorner.getX() < secondCorner.getX()) ? firstCorner.getX() : secondCorner.getX();
	int maxX = (firstCorner.getX() > secondCorner.getX()) ? firstCorner.getX() : secondCorner.getX();
	int minY = (firstCorner.getY() < secondCorner.getY()) ? firstCorner.getY() : secondCorner.getY();
	int maxY = (firstCorner.getY() > secondCorner.getY()) ? firstCorner.getY() : secondCorner.getY();

	Point s = getSize();
	int halfWidth = s.getX()/2;
	int halfHeight = s.getY()/2;

	return (x >= minX - halfWidth)
		&& (x <= maxX + halfWidth)
		&& (y >= minY - halfHeight)
		&& (y <= maxY + halfHeight);

}

// QC:P
bool Collidable::hasPerspective() {
	if((getLeftmostPoint() == NULL) || (getRightmostPoint() != NULL))
		return false;
	
	return (getLeftmostPoint()->getX() != getRightmostPoint()->getX())
	|| (getLeftmostPoint()->getY() != getRightmostPoint()->getY());
}

// QC:P
void Collidable::enableCollisions() {
	collisionsEnabled = true;
}

// QC:P
void Collidable::disableCollisions() {
	collisionsEnabled = false;
}

// QC:P
bool Collidable::getCollisionsEnabled() {
	return collisionsEnabled;
}

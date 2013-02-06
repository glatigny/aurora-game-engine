#include "sprite.h"
#include "state.h"
#include <iostream>
#define HEADER sprite
#define HELPER scriptable_object_cpp
#include "include_helper.h"

Sprite::Sprite(State* attachedState, Translatable* offset, GraphicalEffects* effects) : Translatable(offset), Entity(attachedState), Renderable(attachedState, offset, effects, 0) {
	currentStance = NULL;
}

Sprite::~Sprite() {
	for(std::map<const char*, Stance*, strCmp>::iterator stance = stances.begin(); stance != stances.end(); stance++) {
		AOEFREE((void*)(stance->first));
		delete stance->second;
	}
}

void Sprite::setStance(const char* newStance) {
	xerror(newStance, "Cannot switch to a NULL stance.");
	xerror(stances.find(newStance) != stances.end(), "Cannot find stance %s.", newStance);

	currentStance = stances[newStance];
}

bool Sprite::exist(const char* stanceName) {
	return (stances.find(stanceName) != stances.end());
}

bool Sprite::isIn(const char* stanceName) {
	xerror(stances.find(stanceName) != stances.end(), "Cannot find stance %s.", stanceName);
	return stances.find(stanceName)->second == currentStance;
}

Stance* Sprite::getCurrentStance() {
	assert(currentStance != NULL);

	return currentStance;
}

void Sprite::addStance(const char* stanceName, Stance* stance) {
	xerror(stanceName, "Trying to add a stance with a NULL name.");
	xerror(stance, "Adding a NULL stance %s to a sprite.", stanceName);

	stance->setParent(this);

	stances[AOESTRDUP(stanceName)] = stance;

	if(currentStance == NULL) {
		currentStance = stance;
	}
}

void Sprite::updateObject(dur elapsedTime) {
	xerror(currentStance != NULL, "Sprite has no stance.");
	currentStance->update(elapsedTime);
}

void Sprite::renderGraphics() {
	xerror(currentStance != NULL, "Sprite has no stance.");
	currentStance->render();
}

void Sprite::loadGraphics() {
	for(std::map<const char*, Stance*, strCmp>::iterator stance = stances.begin(); stance != stances.end(); stance++) {
		stance->second->loadGraphics();
	}
}

void Sprite::unloadGraphics() {
	for(std::map<const char*, Stance*, strCmp>::iterator stance = stances.begin(); stance != stances.end(); stance++) {
		stance->second->unloadGraphics();
	}
}

bool Sprite::inside(Translatable& firstCorner, Translatable& secondCorner) {
	assert(currentStance != NULL);
	
	Point finalFirstCorner(firstCorner.getX() - getX(), firstCorner.getY() - getY());
	Point finalSecondCorner(secondCorner.getX() - getX(), secondCorner.getY() - getY());

	return currentStance->inside(finalFirstCorner, finalSecondCorner);
}

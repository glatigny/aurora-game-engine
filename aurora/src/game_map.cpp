#include "game_map.h"
#include "state.h"
#define HEADER game_map
#define HELPER scriptable_object_cpp
#include "include_helper.h"
#include "object_sorting.h"
#include <algorithm>

// QC:W
GameMap::GameMap(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, int collisionLayers, VFS::File* script, Translatable* topLeft, Translatable* bottomRight) : Translatable(offset), Entity(attachedState), Collidable(attachedState, offset, effects, layer, collisionLayers), SubScriptable(attachedState->script(), script) {
	assert(topLeft);
	assert(bottomRight);
	
	coord x1 = topLeft->getX();
	coord x2 = bottomRight->getX();
	coord y1 = topLeft->getY();
	coord y2 = bottomRight->getY();
	
	ptTopLeft.setPosition( MIN(x1, x2), MIN(y1, y2) );
	ptBottomRight.setPosition( MAX(x1, x2), MAX(y1, y2) );
	
}

GameMap::~GameMap() {
	for(ObjectList::iterator i = objects.begin(); i != objects.end(); i++) {
		delete *i;
	}
}

bool GameMap::isInMap(Collidable* obj) {
// QC:?
	for(ObjectList::iterator i = objects.begin(); i != objects.end(); i++) {
		if((*i) == obj) {
			return true;
		}
	}
	
	return false;
}

// QC:B (FIXME : l'assertion isInMap a renvoye false)
void GameMap::addObject(Collidable* obj) {
	xerror(obj != NULL, "Trying to add a NULL object to a map.");
	assert(!isInMap(obj));
	
	obj->setParent(this);
	objects.push_back(obj);
}

// QC:P
void GameMap::removeObject(Collidable* obj) {
	assert(obj);
	assert(isInMap(obj));

	for(ObjectList::iterator i = objects.begin(); i != objects.end(); i++) {
		if((*i) == obj) {
			objects.erase(i);
			obj->doom();
			return;
		}
	}

	assert(false);
}

// QC:W (devrait calculer la taille en fonction des objets contenus et des coins)
Point GameMap::getSize() {return Point(0, 0);}

// QC:A (a tracer avec un debugger)
int GameMap::signal(Collidable* other, SignalId signal, int data) {
	if(this == other) {
		return 0;
	}

	int rdata = 0;
	
	Collision c;
	for(ObjectList::iterator i = objects.begin(); i != objects.end(); i++) {
		c = (*i)->hitBy(other);
		if(c.collided()) {
			rdata = (*i)->signal(other, signal, data);
		}
	}

	return rdata;
}

// QC:G (semble OK, mais fait appel a bcp de code peu sur)
Collision GameMap::hitBy(Collidable* other) {
	Collision c;
	if(!collisionsEnabled || this == other) {
		return Collision();
	}
	
	for(ObjectList::iterator i = objects.begin(); i != objects.end(); i++) {
		c = (*i)->hitBy(other);
		if(c.collided()) {
			return c;
		}
	}

	return Collision();
}

// QC:?
void GameMap::applyCollisions() {
	unsigned int size = objects.size();
	for(unsigned int i = 0; i < size - 1; i++) {
		Collidable* first = objects[i];
		for(unsigned int j = i + 1; j < size; j++) {
			Collidable* second = objects[j];
			
			Collision c = first->hitBy(second);
			if(c.collided())
			{
				first->collided(&c);
				c.swap();
				second->collided(&c);
			}
		}
	}
}

// QC:P
void GameMap::updateObject(dur elapsedTime) {
	if(script()->isRunning()) {
		script()->framestep();
	}
	
	// Update objects
	ObjectList objects_before = objects;
	for(ObjectList::iterator i = objects_before.begin(); i != objects_before.end(); i++) {
		(*i)->update(elapsedTime);
	}
	
	for(ObjectList::iterator i = objects_before.begin(); i != objects_before.end(); i++) {
		(*i)->preCollisions(elapsedTime);
	}

	applyCollisions();
	
	for(ObjectList::iterator i = objects_before.begin(); i != objects_before.end(); i++) {
		(*i)->postCollisions(elapsedTime);
	}
}

// QC:P (s'il y a bien une fonction complexe qui fonctionne, c'est celle-ci !)
void GameMap::renderGraphics() {
	// Sort objects from furthest to nearest
	std::sort(objects.begin(), objects.end(), ObjectSorting());
	
	// Render objects
	for(ObjectList::iterator obj = objects.begin(); obj != objects.end(); obj++) {
		(*obj)->render();
	}
}

// QC:P
void GameMap::loadGraphics() {
	for(ObjectList::iterator i = objects.begin(); i != objects.end(); i++) {
		(*i)->loadGraphics();
	}
}

// QC:P
void GameMap::unloadGraphics() {
	for(ObjectList::iterator i = objects.begin(); i != objects.end(); i++) {
		(*i)->unloadGraphics();
	}
}


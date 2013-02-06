#ifndef GAME_MAP_H
#define GAME_MAP_H

class State;

#include "resizable.h"
#include "collidable.h"
#include "sub_scriptable.h"
#include "signalable.h"
#include <deque>
#include <algorithm>

/** A GameMap is a collection of collidable objects.
 * The map inherits rendering and collisions from its children.
 */
class GameMap : public Collidable, public Resizable, public SubScriptable {
private:
	typedef std::vector<Collidable*> ObjectList;
	ObjectList objects; /** Stores map objects. */
	Point ptTopLeft;
	Point ptBottomRight;
	
	void applyCollisions();
	
public:
#define HEADER game_map
#define HELPER scriptable_object_h
#include "include_helper.h"
	GameMap() {}
	GameMap(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, int collisionLayers, VFS::File* script, Translatable* topLeft, Translatable* bottomRight);
	virtual ~GameMap();

	void addObject(Collidable* object);
	void removeObject(Collidable* object);

	Translatable* getPtTopLeft() {
		return &ptTopLeft;
	}

	Translatable* getPtBottomRight() {
		return &ptBottomRight;
	}
	
	bool isInMap(Collidable* obj);

	/* Collidable */

	virtual int signal(Collidable* other, SignalId signal, int data);
	
	virtual Translatable* getLeftmostPoint() {
		return NULL;
	}
	
	virtual Translatable* getRightmostPoint() {
		return NULL;
	}

	/** Get the size of the object.
	 * This function returns the size of a rectangle that fits entirely the object. It is not necessarily the size on screen nor the size of the texture to be displayed, but such behaviour is recommended.
	 * @return the size of a rectangle that bounds this object.
	 */
	virtual Point getSize();
	virtual Collision hitBy(Collidable* other);

	/* Renderable */

	virtual void renderGraphics();
	virtual void loadGraphics();
	virtual void unloadGraphics();

	/* Blocking */

	virtual bool isFinished() {
		return Renderable::isFinished() && SubScriptable::isFinished();
	}

	bool wait() { return !isFinished(); }

	/* Realtime */

	virtual void updateObject(dur elapsedTime);

};

#endif


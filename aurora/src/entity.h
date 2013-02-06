#ifndef ENTITY_H
#define ENTITY_H

#include "aoe_object.h"

class State;

/** Entity is the main interface for every tangible object.
 */
class Entity : virtual public AOEObject {
protected:
	State* attachedState;

public:
	Entity() : attachedState(NULL) {}

	Entity(State* newAttachedState) : attachedState(newAttachedState) {
		xassert(attachedState, "Object %p is not attached to any state.", (AOEObject*)this);
	}

	/** Gets the container state of this object.
	 * Under normal operation, it is always valid, never NULL and does not need to be checked.
	 * Remember that update() checks for nullity before calling updateObject and other update passes, so checking the state pointer is never useful.
	 */
	State* state() {
		return attachedState;
	}

	void setAttachedState(State* newState);

	/* AOEObject */

	AuroraEngine* engine();
	VFS::File* source();
	virtual ~Entity() { }
};
#endif


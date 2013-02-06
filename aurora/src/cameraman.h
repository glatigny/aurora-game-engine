#ifndef CAMERAMAN_H
#define CAMERAMAN_H

#include "state.h"
#include "realtime.h"

/** A cameraman is a class that defines the behaviour of the camera.
 * It is allowed to move the map of the state to keep elements centered.
 *
 * TODO : Derive from Realtime
 */
class Cameraman : public ScriptableObject, public Realtime {
public:
#define HEADER cameraman
#define HELPER scriptable_object_h
#include "include_helper.h"
	virtual ~Cameraman() {}

	Cameraman() {}
	Cameraman(State* attachedState);

	/** Update the cameraman's position.
	 * @param elapsedTime the amount of time passed since last call to this function.
	 */
	virtual void update(dur elapsedTime) = 0;

	/** Get the position of the camera.
	 * @return the position of the camera.
	 */
	virtual Point getCameraPosition() = 0;

	/* Blocking */

	virtual bool isFinished() {
		return false;
	}

	/* AOEObject */

	virtual VFS::File* source() { return ScriptableObject::source(); }
};

#endif /* CAMERAMAN_H */


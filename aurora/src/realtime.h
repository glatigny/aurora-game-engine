#ifndef REALTIME_H
#define REALTIME_H

class State;

#include "entity.h"
#include "restartable.h"
#include "blocking.h"

#ifndef NDEBUG
#include "vfs.h"
#endif

/** An interface for objects that receive periodic updates.
 * Examples are animations, sound envelopes, script objects, etc.
 * The interface also defines a way to reset the internal notion of time for objects.
 */
class Realtime : virtual public Entity, public Restartable, public Blocking {
protected:
	bool updateEnabled;
	dur playbackSpeed;

public:
	Realtime() : updateEnabled(true), playbackSpeed(100) {}
	Realtime(State* attachedState);
	virtual ~Realtime();

	/** Update the status of this object after some time passed.
	 * This function is called by the engine to update the status of this object.
	 * @param elapsedTime the number of milliseconds between this call and the previous one. It can be 0 if the time is unknown.
	 */
	void update(dur elapsedTime) {
		if(!updateEnabled || !playbackSpeed)
			return;

		if(playbackSpeed != 100) {
			elapsedTime = elapsedTime * playbackSpeed / 100;
		}

		xassert(state(), "Object %p(%s) is not attached to any state.", (AOEObject*)this, source()->name());
		
		// Update phases :

		updateOverlay(elapsedTime);

		if(!updateSubstitute(elapsedTime))
			return;

		updateObject(elapsedTime);

		if(!updatePreScript(elapsedTime))
			return;

		if(!updateDynamics(elapsedTime))
			return;

		updatePostScript(elapsedTime);

		updateUnderlay(elapsedTime);
	}

	void disableUpdates() {
		updateEnabled = false;
	}

	void enableUpdates() {
		updateEnabled = true;
	}

	/** Set playback speed globally.
	 */
	virtual void setPlaybackSpeed(dur newPlaybackSpeed) { playbackSpeed = newPlaybackSpeed; }

	/** Update phase for overlaid objects.
	 * Called by update()
	 */
	virtual void updateOverlay(dur elapsedTime) {}

	/** Update phase for substitutes.
	 * Called by update()
	 * @return true to continue processing, false to interrupt it.
	 */
	virtual bool updateSubstitute(dur elapsedTime) { return true; }

	/** Update phase for pre-update scripts.
	 * Called by update()
	 * @return true to continue processing, false to interrupt it.
	 */
	virtual bool updatePreScript(dur elapsedTime) { return true; }

	/** Update phase for internal object status.
	 * Called by update()
	 */
	virtual void updateObject(dur elapsedTime) {}

	/** Update phase for dynamics.
	 * Called by update()
	 */
	virtual bool updateDynamics(dur elapsedTime) { return true; }

	/** Update phase for post processing scripts.
	 * Called by update()
	 */
	virtual void updatePostScript(dur elapsedTime) {}

	/** Update phase for underlaid objects.
	 * Called by update()
	 */
	virtual void updateUnderlay(dur elapsedTime) {}
};

#endif /* REALTIME_H */

#ifndef SCRIPT_TRIGGER_H
#define SCRIPT_TRIGGER_H

#include "game_object.h"
#include "sub_scriptable.h"

/** A ScriptTrigger is a collision zone which starts a script if collided.
 * The function "hit(other)" is called in case of collision.
 */
class ScriptTrigger : public GameObject, public SubScriptable {
private:
	Renderable* attachedObject;
public:
#define HEADER script_trigger
#define HELPER scriptable_object_h
#include "include_helper.h"
	ScriptTrigger() {}
	ScriptTrigger(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, int collisionLayers, Collidable* collision, VFS::File* script, Renderable* attachedObject);
	virtual ~ScriptTrigger() {}

	/** Blocking function which manages hits.
	 * This function is called whenever something hits the ScriptTrigger.
	 * @param other Object which has hit this one.
	 * @return true to continue this function and disable any subsequent hit, false to return to normal state.
	 */
	bool onHit(ScriptableObject* other);

	int onSignal(ScriptableObject* other, SignalId signalId, int parameter);

	/* Signalable */

	int signal(ScriptableObject* other, SignalId signalId, int parameter);

	/* Collidable */

	virtual Collision hitBy(Collidable* other);

	/* Renderable */

	virtual void renderGraphics() {}
	virtual void loadGraphics();
	virtual void unloadGraphics();

	/* Realtime */

	virtual void updateObject(dur elapsedTime);
};

#endif


#include "state.h"
#include "script_trigger.h"

#define HEADER script_trigger
#define HELPER scriptable_object_cpp
#include "include_helper.h"


ScriptTrigger::ScriptTrigger(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, int collisionLayers, Collidable* collision, VFS::File* script, Renderable* newAttachedObject) : Entity(attachedState), Translatable(offset), GameObject(attachedState, offset, effects, layer, collisionLayers, collision), SubScriptable(state()->script(), script) {
	attachedObject = newAttachedObject;
	if(attachedObject) {
		attachedObject->setParent(this);
	}
}

/*------------------------------------------------------------------------------*/

bool ScriptTrigger::onHit(ScriptableObject* other) {
	return false;
}

/*------------------------------------------------------------------------------*/

Collision ScriptTrigger::hitBy(Collidable* other) {
	Collision c;
	
	c = GameObject::hitBy(other);
	if(!script()->isRunning()) {
		if(c.collided()) {
			// Call onHit via the script engine.
			script()->callFunction(*this, (char*)"onHit", "o", (ScriptableObject*)other);
		}
	}

	// No collision if script is running
	return c;
}
	
int ScriptTrigger::onSignal(ScriptableObject* other, SignalId signalId, int parameter) {
	return 0;
}

/*------------------------------------------------------------------------------*/

int ScriptTrigger::signal(ScriptableObject* other, SignalId signalId, int data) {
#if defined(SYSLOG_VERBOSE) && SYSLOG_VERBOSE >= 3
	syslog("ScriptTrigger received a signal");
#endif
	script()->callFunction(*this, (char*)"onSignal", "oii", other, signalId, data);
	return script()->threadsRunning()?2:1;
}

/*------------------------------------------------------------------------------*/

void ScriptTrigger::updateObject(dur elapsedTime)
{
	if(attachedObject) {
		setPosition(*attachedObject);
	}

	// FIXME : move to script update
	if(script()->isRunning()) {
		script()->framestep();
	}
}

/*------------------------------------------------------------------------------*/

void ScriptTrigger::loadGraphics()
{
	if(attachedObject)
		attachedObject->loadGraphics();
}

/*------------------------------------------------------------------------------*/

void ScriptTrigger::unloadGraphics()
{
	if(attachedObject)
		attachedObject->unloadGraphics();
}


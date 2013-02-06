#ifndef SCRIPTABLE_PARTICLE_H
#define SCRIPTABLE_PARTICLE_H

#include "flying_particle.h"
#include "sub_scriptable.h"

/** A particle which is controlled by script.
 * The step() function is not called directly : it is instead called via
 * a redirectable callFunction().
 *
 * VIRTUAL CONSTRUCTORS:
 *  - Translatable(offset)
 */
class ScriptableParticle : public FlyingParticle, public SubScriptable {
protected:
	char* defaultFunctionName;
	char* steppingFunctionName;
public:
#define HEADER scriptable_particle
#define HELPER scriptable_object_h
#include "include_helper.h"
	ScriptableParticle() {}
	ScriptableParticle(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, int collisionLayers, Collidable* collision, Renderable* frame, int direction, int speed, int lifespan, VFS::File* script);
	virtual ~ScriptableParticle() {}

	/* Realtime */

	virtual bool updatePreScript(dur elapsedTime);
};

#endif /* SCRIPTABLE_PARTICLE_H */


#include "scriptable_particle.h"
#include "state.h"
#define HEADER scriptable_particle
#define HELPER scriptable_object_cpp
#include "include_helper.h"

ScriptableParticle::ScriptableParticle(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, int collisionLayers, Collidable* collision, Renderable* frame, int direction, int speed, int lifespan, VFS::File* scriptFileName) : Translatable(offset), FlyingParticle(attachedState, offset, effects, layer, collisionLayers, collision, frame, direction, speed, lifespan, 0, 0, NULL), SubScriptable(attachedState->script(), scriptFileName) {
}

bool ScriptableParticle::updatePreScript(dur elapsedTime) {
	bool runNormalStep = script()->callFunction(*this, "step", elapsedTime).getbool();
	script()->framestep();
	if(!runNormalStep && frame)
		frame->update(elapsedTime);
	return runNormalStep;
}

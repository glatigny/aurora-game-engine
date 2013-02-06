#ifndef EFFECT_KEY_H
#define EFFECT_KEY_H

#include "renderable.h"

/** A key point for effect interpolators.
 * Because effects are linked to Renderables, you cannot specify an effect in XML files.
 * This class is a "null" renderable which does not display anything but only stores
 * its effects and its position.
 */
class EffectKey : public Renderable {
public:
#define HEADER effect_key
#define HELPER scriptable_object_h
#include "include_helper.h"
	EffectKey() {}
	EffectKey(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer) : Translatable(offset), Entity(attachedState), Renderable(attachedState, offset, effects, layer) {}
	virtual ~EffectKey() {}

	/* Renderable */

	virtual void renderGraphics() {}
	virtual void loadGraphics() {}
	virtual void unloadGraphics() {}

	/* Realtime */

	virtual void updateObject(dur elapsedTime) {}

	/* Translatable */

	virtual bool inside(Translatable& firstCorner, Translatable& secondCorner) {return false;}
};
#endif


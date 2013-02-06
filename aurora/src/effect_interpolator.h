#ifndef EFFECT_INTERPOLATOR_H
#define EFFECT_INTERPOLATOR_H

#include "animation.h"

class EffectInterpolator : public Animation {
protected:
	Renderable* interpolatedObject;
public:
#define HEADER effect_interpolator
#define HELPER scriptable_object_h
#include "include_helper.h"
	EffectInterpolator() {}
	EffectInterpolator(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, int frameDuration, int animSpeed, bool reversePlayback, int loopType, int startTime, int endTime, const char* interpolations, Renderable* interpolatedObject);
	virtual ~EffectInterpolator() {}

	/** Returns the currently interpolated object.
	 * @return the currently interpolated object.
	 */
	virtual Renderable* getInterpolatedObject();

	/** Changes the currently interpolated object.
	 * @param newInterpolatedObject the new object to interpolated.
	 */
	virtual void setInterpolatedObject(Renderable* newInterpolatedObject);

	/* Renderable */

	virtual void renderGraphics();
	virtual void loadGraphics();
	virtual void unloadGraphics();

	/* Realtime */

	virtual void updateObject(dur elapsedTime);
};

#endif /* EFFECT_INTERPOLATOR_H */


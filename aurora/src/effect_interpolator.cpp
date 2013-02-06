#include "effect_interpolator.h"
#include "state.h"

#define HEADER effect_interpolator
#define HELPER scriptable_object_cpp
#include "include_helper.h"

#include "renderable.h"

EffectInterpolator::EffectInterpolator(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, int frameDuration, int animSpeed, bool reversePlayback, int loopType, int startTime, int endTime, const char* interpolations, Renderable* newInterpolatedObject) : Translatable(offset), Animation(attachedState, offset, effects, layer, frameDuration, animSpeed, reversePlayback, loopType, startTime, endTime, 1, interpolations) {
	interpolatedObject = NULL;
	setInterpolatedObject(newInterpolatedObject);
}


void EffectInterpolator::setInterpolatedObject(Renderable* newInterpolatedObject) {
	interpolatedObject = newInterpolatedObject;

	if(interpolatedObject) {
		interpolatedObject->setParent(this);
	}
}


Renderable* EffectInterpolator::getInterpolatedObject() {
	return interpolatedObject;
}


void EffectInterpolator::renderGraphics() {
	if(interpolatedObject) {
		// Store the object's properties.
		Point originalPosition(*interpolatedObject);
		BasicGraphicalEffects* originalBasicGraphicalEffects = interpolatedObject->objectEffects.basic;
		GraphicalEffects::CustomEffectMap originalCustomGraphicalEffects = interpolatedObject->objectEffects.custom;
		interpolatedObject->objectEffects.basic = NULL;
		interpolatedObject->objectEffects.custom.clear();

		// Set the interpolated values.
		if(currentInterpolator) {
			interpolatedObject->setPosition(currentInterpolator->interpolatedPosition);
			interpolatedObject->objectEffects = currentInterpolator->interpolatedEffects;
		} else {
			Renderable* currentEffect = frames[currentFrame];

			interpolatedObject->setPosition(*currentEffect);
			interpolatedObject->objectEffects = currentEffect->objectEffects;
		}

		// Render the interpolated object.
		interpolatedObject->render();

		// Put the object back where we found it.
		interpolatedObject->setPosition(originalPosition);
		interpolatedObject->objectEffects.basic = originalBasicGraphicalEffects;
		interpolatedObject->objectEffects.custom = originalCustomGraphicalEffects;
	}
}


void EffectInterpolator::updateObject(dur elapsedTime) {
	if(interpolatedObject) {
		interpolatedObject->update(elapsedTime);
	}
	
	Animation::update(elapsedTime);
}


void EffectInterpolator::loadGraphics() {
	Animation::loadGraphics();
	if(interpolatedObject) {
		interpolatedObject->loadGraphics();
	}
}


void EffectInterpolator::unloadGraphics() {
	Animation::unloadGraphics();
	if(interpolatedObject) {
		interpolatedObject->unloadGraphics();
	}
}


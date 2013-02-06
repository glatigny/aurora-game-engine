#include "renderable_interpolator.h"
#include "interpolation.h"

// QC:?
void InterpolationDescriptor::decodeInterpolationString(const char* interpolationString) {
	// OPOS
	x = NULL; // XPOS
	y = NULL; // YPOS

	// SCAL
	horizontalScale = NULL; // HSCL
	verticalScale = NULL; // VSCL

	rotationAngle = NULL; // ROTA

	opacity = NULL; // OPAC

	sectionX = NULL; // CROP
	sectionY = NULL; // CROP
	sectionW = NULL; // CROP
	sectionH = NULL; // CROP

	frame = NULL; // FRAM
	while(interpolationString && *interpolationString) {
		assert(strlen(interpolationString) >= 5);
		assert(interpolationString[4] == '=');
		uint32_t interpolationID = NAME_TO_ID(interpolationString);
		interpolationString += 5;

		switch(interpolationID) {
#define BUILTIN_EFFECT(n, interpolation) case CONST_NAME_TO_ID(n): { interpolation = newInterpolation(interpolationString);} break;
			BUILTIN_EFFECT('HSCL', horizontalScale)
			BUILTIN_EFFECT('VSCL', verticalScale)
			BUILTIN_EFFECT('ROTA', rotationAngle)
			BUILTIN_EFFECT('OPAC', opacity)
			BUILTIN_EFFECT('XPOS', x)
			BUILTIN_EFFECT('YPOS', y)
			BUILTIN_EFFECT('FRAM', frame)
#undef BUILTIN_EFFECT
			case CONST_NAME_TO_ID('OPOS'): {
				x = newInterpolation(interpolationString);
				y = newInterpolation(interpolationString);
			} break;

			case CONST_NAME_TO_ID('SCAL'): {
				horizontalScale = newInterpolation(interpolationString);
				verticalScale = newInterpolation(interpolationString);
			} break;

			case CONST_NAME_TO_ID('CROP'): {
				sectionX = newInterpolation(interpolationString);
				sectionY = newInterpolation(interpolationString);
				sectionW = newInterpolation(interpolationString);
				sectionH = newInterpolation(interpolationString);
			} break;

#define NOTHING ;

#define AMOUNT \
	custom[interpolationID] = newInterpolation(interpolationString); assert(custom[interpolationID]);
#define INTEGER \
	custom[interpolationID] = newInterpolation(interpolationString); assert(custom[interpolationID]);
#define RGBVALUE \
	customR[interpolationID] = newInterpolation(interpolationString); assert(customR[interpolationID]); \
	customG[interpolationID] = newInterpolation(interpolationString); assert(customG[interpolationID]); \
	customB[interpolationID] = newInterpolation(interpolationString); assert(customB[interpolationID]);
#define ALPHA \
	customA[interpolationID] = newInterpolation(interpolationString); assert(customA[interpolationID]);
#define XY \
	customX[interpolationID] = newInterpolation(interpolationString); assert(customX[interpolationID]); \
	customY[interpolationID] = newInterpolation(interpolationString); assert(customY[interpolationID]);
#define RGBAVALUE RGBVALUE ALPHA
#define RGBA_AMOUNT RGBAVALUE AMOUNT

#define HEADER graphical_effects
#define HELPER custom_interpolation_construction
#include "include_helper.h"

#undef NOTHING
#undef AMOUNT
#undef INTEGER
#undef RGBVALUE
#undef RGBAVALUE
#undef RGBA_AMOUNT
#undef ALPHA
#undef XY
		}
		interpolationString = strchr(interpolationString, ';');

		// Eliminate separator
		if(interpolationString) {
			interpolationString++;
		}
	}
}

// QC:?
void InterpolationDescriptor::setInterpolationBoundaries(dur startTime, dur endTime, Translatable* startPosition, Translatable* endPosition, GraphicalEffects* startGraphicalEffects, GraphicalEffects* endGraphicalEffects) {
	if(x) {
		x->set(startTime, endTime, startPosition->getX(), endPosition->getX(), true);
	}
	if(y) {
		y->set(startTime, endTime, startPosition->getY(), endPosition->getY(), true);
	}
#define BASIC_INTERPOLATION(i, getter) if(i) { i->set(startTime, endTime, startGraphicalEffects->getter, endGraphicalEffects->getter, true);	}
	BASIC_INTERPOLATION(horizontalScale,getHorizontalScale())
	BASIC_INTERPOLATION(verticalScale,getVerticalScale())
	BASIC_INTERPOLATION(rotationAngle,getRotation())
	BASIC_INTERPOLATION(opacity,getOpacity())
	BASIC_INTERPOLATION(sectionX,getSection()->getX())
	BASIC_INTERPOLATION(sectionY,getSection()->getY())
	BASIC_INTERPOLATION(sectionW,getSection()->getW())
	BASIC_INTERPOLATION(sectionH,getSection()->getH())
#undef BASIC_INTERPOLATION

	if(frame) {
		frame->set(startTime, endTime, 0, 255, true);
	}

	if(!customList.empty()) {
		for(CustomEffectList::iterator it = customList.begin(); it != customList.end(); it++) {
			uint32_t interpolationID = *it;

			const CustomGraphicalEffectsValue& startVal = startGraphicalEffects->getCustomEffectValue(interpolationID);
			const CustomGraphicalEffectsValue& endVal = endGraphicalEffects->getCustomEffectValue(interpolationID);

			switch(interpolationID) {
#define NOTHING ;

#define AMOUNT \
				custom[interpolationID]->set(startTime, endTime, startVal.color.amount, endVal.color.amount, true);
#define INTEGER \
				custom[interpolationID]->set(startTime, endTime, startVal.integer, endVal.integer, false);
#define RGBVALUE \
				customR[interpolationID]->set(startTime, endTime, startVal.color.r, endVal.color.r, true); \
				customG[interpolationID]->set(startTime, endTime, startVal.color.g, endVal.color.g, true); \
				customB[interpolationID]->set(startTime, endTime, startVal.color.b, endVal.color.b, true);
#define ALPHA \
				customA[interpolationID]->set(startTime, endTime, startVal.color.a, endVal.color.a, true);
#define XY \
				customX[interpolationID]->set(startTime, endTime, startVal.point.x, endVal.point.x, false); \
				customY[interpolationID]->set(startTime, endTime, startVal.point.y, endVal.point.y, false);

#define RGBAVALUE RGBVALUE ALPHA
#define RGBA_AMOUNT RGBAVALUE AMOUNT

#define HEADER graphical_effects
#define HELPER effect_set
#include "include_helper.h"

#undef NOTHING
#undef AMOUNT
#undef INTEGER
#undef RGBVALUE
#undef RGBAVALUE
#undef RGBA_AMOUNT
#undef ALPHA
#undef XY
			}
		}
	}
}

// QC:?
void InterpolationDescriptor::applyTo(Renderable* target, dur time, int frameBlend)
{
	if(x)
		target->setX(x->getInterpolatedValue(time));
	
	if(y)
		target->setY(y->getInterpolatedValue(time));

	GraphicalEffects* fx = target->getObjectGraphicalEffects();
	xassert(fx, "Renderable has no graphical effects");
#define INTERPOLATE(interpolation, setter) if(interpolation) { fx->setter(interpolation->getInterpolatedValue(time)); }
	INTERPOLATE(horizontalScale, setHorizontalScale)
	INTERPOLATE(verticalScale, setVerticalScale)
	INTERPOLATE(rotationAngle, setRotation)

	// TODO : implement section interpolation
	
	if(frame) {
		int o = opacity?opacity->getInterpolatedValue(time):255;
		fx->setOpacity(o * frameBlend / 255);
	} else {
		INTERPOLATE(opacity, setOpacity)
	}
#undef INTERPOLATE

	if(!customList.empty()) {
		for(CustomEffectList::iterator it = customList.begin(); it != customList.end(); it++) {
			uint32_t interpolationID = *it;

			switch(interpolationID) {

#define NOTHING ;

#define AMOUNT \
				fx->custom[interpolationID].color.amount = custom[interpolationID]->getInterpolatedValue(time);
#define INTEGER \
				fx->custom[interpolationID].integer = custom[interpolationID]->getInterpolatedValue(time);
#define RGBVALUE \
				fx->custom[interpolationID].color.r = customR[interpolationID]->getInterpolatedValue(time); \
				fx->custom[interpolationID].color.g = customG[interpolationID]->getInterpolatedValue(time); \
				fx->custom[interpolationID].color.b = customB[interpolationID]->getInterpolatedValue(time);
#define ALPHA \
				fx->custom[interpolationID].color.a = customA[interpolationID]->getInterpolatedValue(time);
#define XY \
				fx->custom[interpolationID].point.x = customX[interpolationID]->getInterpolatedValue(time); \
				fx->custom[interpolationID].point.y = customY[interpolationID]->getInterpolatedValue(time);

#define RGBAVALUE RGBVALUE ALPHA
#define RGBA_AMOUNT RGBAVALUE AMOUNT

#define HEADER graphical_effects
#define HELPER effect_set
#include "include_helper.h"

#undef NOTHING
#undef AMOUNT
#undef INTEGER
#undef RGBVALUE
#undef RGBAVALUE
#undef RGBA_AMOUNT
#undef ALPHA
#undef XY
			}
		}
	}
}

// QC:P
RenderableInterpolator::RenderableInterpolator(Renderable* start, Renderable* end, dur startTime, int newDuration, const char* interpolationString) :
	startFrame(start),
	endFrame(end),
	duration(newDuration),
	timePosition(startTime),
	interpolations(interpolationString),
	startFramePosition(startFrame),
	endFramePosition(endFrame),
	startFrameEffects(*startFrame->getObjectGraphicalEffects()),
	endFrameEffects(*endFrame->getObjectGraphicalEffects())
{
	assert(startFrame);
	assert(endFrame);
	startFrame->setTempParent(this);
	endFrame->setTempParent(this);
	this->attachToState(start->state());
	if(duration < 0) {
		duration = -duration;
	}

	interpolations.setInterpolationBoundaries(startTime, startTime + duration, startFrame, endFrame, startFrame->getObjectGraphicalEffects(), endFrame->getObjectGraphicalEffects());

}

RenderableInterpolator::~RenderableInterpolator()
{
}

// QC:?
void RenderableInterpolator::resetChildren() {
	startFrame->setPosition(startFramePosition);
	endFrame->setPosition(endFramePosition);
	*startFrame->getObjectGraphicalEffects() = startFrameEffects;
	*endFrame->getObjectGraphicalEffects() = endFrameEffects;
}

// QC:P
void RenderableInterpolator::setChildrenParent(Renderable* newParent) {
	startFrame->setTempParent(newParent);
	endFrame->setTempParent(newParent);
}

// QC:P
bool RenderableInterpolator::isFinished() {
	return Renderable::isFinished() || timePosition >= duration;
}

// QC:P
void RenderableInterpolator::loadGraphics() {
	startFrame->loadGraphics();
	if(interpolations.frame)
		endFrame->loadGraphics();
}

// QC:P
void RenderableInterpolator::unloadGraphics() {
	startFrame->unloadGraphics();
	if(interpolations.frame)
		endFrame->unloadGraphics();
}

// QC:P
void RenderableInterpolator::updateObject(dur elapsedTime) {
	startFrame->update(elapsedTime);
	endFrame->update(elapsedTime);

	setTimePosition(timePosition + elapsedTime);
}

// QC:?
void RenderableInterpolator::render() {
	startFrame->render();
	if(interpolations.frame)
		endFrame->render();
}


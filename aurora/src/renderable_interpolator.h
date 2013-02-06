#ifndef RENDERABLE_INTERPOLATOR_H
#define RENDERABLE_INTERPOLATOR_H

#include "renderable.h"
#include "interpolation.h"
#include <map>
#include <string>

class EffectInterpolator;

class InterpolationDescriptor {
private:
	void decodeInterpolationString(const char* interpolationString);

	// QC:?
	void freeInterpolations() {
		if(x) delete x;
		if(y) delete y;
		if(horizontalScale) delete horizontalScale;
		if(verticalScale) delete verticalScale;
		if(rotationAngle) delete rotationAngle;
		if(opacity) delete opacity;
		if(sectionX) delete sectionX;
		if(sectionY) delete sectionY;
		if(sectionW) delete sectionW;
		if(sectionH) delete sectionH;
		if(frame) delete frame;

#define DELETE_CUSTOM(m) for(std::map<uint32_t, Interpolation*>::iterator i = m.begin(); i != m.end(); i++) { delete i->second; }
		DELETE_CUSTOM(custom)
		DELETE_CUSTOM(customR)
		DELETE_CUSTOM(customG)
		DELETE_CUSTOM(customB)
		DELETE_CUSTOM(customA)
		DELETE_CUSTOM(customX)
		DELETE_CUSTOM(customY)
#undef DELETE_CUSTOM
	}

public:
	Interpolation* x;
	Interpolation* y;
	Interpolation* horizontalScale;
	Interpolation* verticalScale;
	Interpolation* rotationAngle;
	Interpolation* opacity;
	Interpolation* sectionX;
	Interpolation* sectionY;
	Interpolation* sectionW;
	Interpolation* sectionH;

	Interpolation* frame;

	int constantOpacity;

	std::map<uint32_t, Interpolation*> custom;
	std::map<uint32_t, Interpolation*> customR;
	std::map<uint32_t, Interpolation*> customG;
	std::map<uint32_t, Interpolation*> customB;
	std::map<uint32_t, Interpolation*> customA;
	std::map<uint32_t, Interpolation*> customX;
	std::map<uint32_t, Interpolation*> customY;

	typedef std::vector<uint32_t> CustomEffectList;
	CustomEffectList customList;

	InterpolationDescriptor() {
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
	}

	// QC:P
	/** Create from an interpolation string.
	 * The interpolation string has the following format :
	 *  "NAME=interpolator:parameter:parameter;NAME=interpolator;..."
	 * NAME is always 4 characters long.
	 */
	InterpolationDescriptor(const char* interpolationString) {
		decodeInterpolationString(interpolationString);
	}

	// QC:P
	~InterpolationDescriptor() {
		freeInterpolations();
	}

	// QC:P
	void setInterpolationString(const char* newInterpolationString) {
		freeInterpolations();
		decodeInterpolationString(newInterpolationString);
	}

	/** Set the boundaries of interpolations.
	 * This function sets the boundaries of all defined interpolators. Values are set to range from one position and one graphical effect to the other one.
	 * This function must be called before querying interpolators status.
	 * @param startTime the time position at which the transition starts. All points before or at startTime will return startPosition and startGraphicalEffect.
	 * @param endTime the time position at which the transition ends. All points after ot at endTime will return endPosition and endGraphicalEffect.
	 * @param startPosition the position associated to startTime.
	 * @param endPosition the position associated to endTime.
	 * @param startGraphicalEffects the effects associated to startTime.
	 * @param endGraphicalEffects the effects associated to endTime.
	 */
	void setInterpolationBoundaries(dur startTime, dur endTime, Translatable* startPosition, Translatable* endPosition, GraphicalEffects* startGraphicalEffects, GraphicalEffects* endGraphicalEffects);


	/** Set the state of a renderable from the current state of this interpolation.
	 * @param target the Renderable to alter.
	 * @param time the index position of the interpolation.
	 */
	void applyTo(Renderable* target, dur time, int frameBlend = 255);
};


class RenderableInterpolator: public Renderable {
	friend class EffectInterpolator;
private:
	Renderable* startFrame;
	Renderable* endFrame;

	int duration;
	int timePosition;

	Point interpolatedPosition;
	GraphicalEffects interpolatedEffects;

	InterpolationDescriptor interpolations;

	// Initial objects state
	Point startFramePosition;
	Point endFramePosition;
	GraphicalEffects startFrameEffects;
	GraphicalEffects endFrameEffects;

public:
	RenderableInterpolator(Renderable* start, Renderable* end, dur startTime, int duration,
	                       const char* interpolationString);
	~RenderableInterpolator();

	void setTimePosition(int newTimePosition) {
		timePosition = newTimePosition;

		if(interpolations.frame) {
			if(!interpolations.opacity) {
				interpolations.applyTo(startFrame, timePosition, (255 - (interpolations.frame)->getInterpolatedValue(timePosition)) * startFrameEffects.getOpacity() / 255);
				interpolations.applyTo(endFrame, timePosition, (interpolations.frame)->getInterpolatedValue(timePosition) * endFrameEffects.getOpacity() / 255);
			} else {
				interpolations.applyTo(startFrame, timePosition, 255 - (interpolations.frame)->getInterpolatedValue(timePosition));
				interpolations.applyTo(endFrame, timePosition, (interpolations.frame)->getInterpolatedValue(timePosition));
			}
		} else {
			interpolations.applyTo(startFrame, timePosition);
		}

		startFrame->invalidateFinalCache();
		endFrame->invalidateFinalCache();
	}

	/** Reset modifications applied to children.
	 * You must call this function before destruction if you plan to reuse children in another object.
	 */
	void setChildrenParent(Renderable* parent);

	void resetChildren();

	Renderable* start() { return startFrame; }
	Renderable* end() { return endFrame; }

	/* Renderable */

	virtual void render();
	virtual void renderGraphics() {
		assert(false);
	}
	virtual void loadGraphics();
	virtual void unloadGraphics();

	/* Realtime */

	virtual void updateObject(dur elapsedTime);

	/* Blocking */

	virtual bool isFinished();

	/* Restartable */

	virtual void restart() {
		if(startFrame)
			startFrame->restart();
//XXX		if(endFrame)
//			endFrame->restart();
	}

	/* Translatable */

	virtual bool inside(Translatable& firstCorner, Translatable& secondCorner) {
		return false; /*TODO implement inside */
	}
};

#endif


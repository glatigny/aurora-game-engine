#ifndef ANIMATION_H
#define ANIMATION_H

#include "renderable.h"
#include "renderable_interpolator.h"
#include "playback_track.h"

#define ANIMATION_RESTART_FRAMES 0
#define ANIMATION_UPDATE_ONE_FRAME 1
#define ANIMATION_UPDATE_ALL_FRAMES 2

/** An animation is a succession of images played at a constant rate.
 */
class Animation : public PlaybackTrack, public Renderable {
protected:
	// Frame cache to regenerate interpolator only if needed.
	int currentFrame;
	int nextFrame;
	int frameUpdateMode;
	std::vector<Renderable*> frames;
	char* interpolations;
	RenderableInterpolator* currentInterpolator;
	Renderable* frame;

private:
	bool autoBoundaries;

	/**
	 */
	void freeInterpolator();


public:
#define HEADER animation
#define HELPER scriptable_object_h
#include "include_helper.h"
	Animation();
	Animation(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, int frameDuration, int animSpeed, bool reversePlayback, int loopType, int startTime, int endTime, int frameUpdateMode, const char* interpolations);
	virtual ~Animation();

	/** Adds a frame to this animation.
	 * @param frame the frame to add to this animation.
	 */
	void addFrame(Renderable* newFrame);

	/** Get a frame from this animation.
	 * Bound checking is forced by an assertion.
	 * @param position the position of the frame in the animation.
	 * @return the frame at the given position.
	 */
	Renderable* getFrame(unsigned int position);

	/* Renderable */

	virtual void renderGraphics();
	virtual void updateObject(dur elapsedTime);
	virtual void loadGraphics();
	virtual void unloadGraphics();

	/* Blocking */

	virtual bool isFinished() {
		return Renderable::isFinished() && PlaybackTrack::isFinished();
	}

	/* Restartable */

	/** Restarts the animation immediatly.*/
	virtual void restart();

	/* Translatable */

	virtual bool inside(Translatable& firstCorner, Translatable& secondCorner);

};

#endif


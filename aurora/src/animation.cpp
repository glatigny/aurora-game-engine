#include "animation.h"
#include "state.h"
#include "decoration.h"
#include "frame.h"
#define HEADER animation
#define HELPER scriptable_object_cpp
#include "include_helper.h"

Animation::Animation() :
	interpolations(NULL), currentInterpolator(NULL), frame(NULL)
{
}

Animation::Animation(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, int newFrameDuration, int newAnimSpeed,
                     bool newReversePlayback, int newLoopType, int newStartTime, int newEndTime, int newFrameUpdateMode,
                     const char* newInterpolations) :
	Entity(attachedState), Translatable(offset),
	PlaybackTrack(0, newFrameDuration, newLoopType, newStartTime, newEndTime, newReversePlayback, newAnimSpeed),
	Renderable(attachedState, offset, effects, layer), currentFrame(0), nextFrame(0), frameUpdateMode(newFrameUpdateMode),
	currentInterpolator(NULL),
	frame(NULL),
	autoBoundaries(loopType == PLAYBACK_ONCE && newEndTime == 0)
{
	if(newInterpolations) {
		interpolations = AOESTRDUP(newInterpolations);
	} else {
		interpolations = NULL;
	}
}

// QC:?
Animation::~Animation() {
	for(std::vector<Renderable*>::iterator it = frames.begin(); it != frames.end(); it++) {
		delete *it;
	}
	freeInterpolator();
	if(interpolations)
		AOEFREE(interpolations);
}

// QC:P
void Animation::addFrame(Renderable* newFrame) {
	assert(newFrame != NULL);

	if(autoBoundaries && duration) {
		end += period;
		targetEnd += period;
	}

	duration += period;

	if((int) frames.size() == currentFrame) {
		frame = newFrame;
	}

	newFrame->setParent(this);
	frames.push_back(newFrame);

	finished = false;
}

// QC:
Renderable* Animation::getFrame(unsigned int position) {
	assert( position < frames.size() );

	return frames[position];
}

// QC:P
void Animation::freeInterpolator() {
	// free current interpolator if necessary
	if(currentInterpolator) {
		delete currentInterpolator;
		currentInterpolator = NULL;
	}
}

// QC:P
inline void Animation::restart() {
	PlaybackTrack::restart();
	for(std::vector<Renderable*>::iterator frame = frames.begin(); frame != frames.end(); frame++) {
		(*frame)->restart();
	}
}

// QC:A
void Animation::updateObject(dur elapsedTime) {
	int cf = currentFrame;
	int nf = nextFrame;

	int tc, tn; // Time from current / time to next

	if(duration == 0) {
		return;
	}

	PlaybackTrack::updateObject(elapsedTime);

	getSurroundingKeys(currentFrame, nextFrame, tc, tn);
	tc = abs(tc);
	tn = abs(tn);

	if(tc < elapsedTime) {
		if(frame && frameUpdateMode != ANIMATION_UPDATE_ALL_FRAMES) {
			frame->update(elapsedTime - tc); // Approximative. Should compute all intermediate frames to update them.
		}
	}

	// Update currentInterpolator
	if(interpolations) {
		if(!currentInterpolator || cf != currentFrame || nf != nextFrame) {
			if(currentInterpolator) {
				currentInterpolator->setChildrenParent(this);
				currentInterpolator->resetChildren();
				delete currentInterpolator;
			}

			if(currentFrame != nextFrame) {
				currentInterpolator = new RenderableInterpolator(frames[currentFrame], frames[nextFrame], 0, tc + tn, interpolations);
				currentInterpolator->setParent(this);
				frame = currentInterpolator;
			} else {
				currentInterpolator = NULL;
				frame = frames[currentFrame];
			}
			invalidateRenderingCache();
		}
	} else {
		if(currentInterpolator) {
			delete currentInterpolator;
			currentInterpolator = NULL;
		}

		if(cf != currentFrame) {
			frame = frames[currentFrame];
			invalidateRenderingCache();
		}
	}

	// Manage frame restart
	if(frame) {
		switch(frameUpdateMode){
		default:
		case ANIMATION_RESTART_FRAMES: {
			if(elapsedTime && (tc <= elapsedTime)) {
				frame->restart();
			}

			frame->update(tc);
		}
			break;

		case ANIMATION_UPDATE_ONE_FRAME: {
			frame->update(tc);
		}
			break;

		case ANIMATION_UPDATE_ALL_FRAMES: {
			for(std::vector<Renderable*>::iterator f = frames.begin(); f != frames.end(); f++) {
				(*f)->update(elapsedTime);
			}
		}
			break;

		}

		if(currentInterpolator == frame) {
			// update messes the interpolator playback pointer.
			// we correct it permanently.
			currentInterpolator->setTimePosition(tc);
		}
	}
}

// QC:P
void Animation::loadGraphics() {
	for(std::vector<Renderable*>::iterator frame = frames.begin(); frame != frames.end(); frame++) {
		(*frame)->loadGraphics();
	}
}

// QC:P
void Animation::unloadGraphics() {
	for(std::vector<Renderable*>::iterator frame = frames.begin(); frame != frames.end(); frame++) {
		(*frame)->unloadGraphics();
	}
}

// QC:P
void Animation::renderGraphics() {
	if(frame) {
		frame->render();
	}
}

// QC:U
bool Animation::inside(Translatable& firstCorner, Translatable& secondCorner) {
	if(!frame) {
		return false;
	}

	Point finalFirstCorner(firstCorner.getX() - getX(), firstCorner.getY() - getY());
	Point finalSecondCorner(secondCorner.getX() - getX(), secondCorner.getY() - getY());

	return frame->inside(finalFirstCorner, finalSecondCorner);
}


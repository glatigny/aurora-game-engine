#include "key_animation.h"
#include "state.h"

#define HEADER key_animation
#define HELPER scriptable_object_cpp
#include "include_helper.h"

KeyAnimation::KeyAnimation(State* attachedState, Translatable* offset, GraphicalEffects* effects, int layer, int newFrameDuration, int newAnimSpeed, bool newReversePlayback, int newLoopType, int newMode, int newFrameUpdateMode, const char* newInterpolations) : Entity(attachedState), Translatable(offset), Animation(attachedState, offset, effects, layer, newFrameDuration, newAnimSpeed, newReversePlayback, newLoopType, 0, 0, newFrameUpdateMode, newInterpolations) {
	setPlaybackMode(newMode);
	finished = true;
}

KeyAnimation::~KeyAnimation() {
	for(std::map<const char*, int, strCmp>::iterator key = keys.begin(); key != keys.end(); key++) {
		AOEFREE((void*)(key->first));
	}
}

// QC:G
void KeyAnimation::playTo(const char* keyName) {
	assert(keyName != NULL);
	assert(keys.find(keyName) != keys.end());

	int pos = keys[keyName];
	int a = currentPosition;
	int b = pos;

	reverse = false;

	switch( mode )
	{
		case KEY_ANIMATION_FORWARD:
			break;

		case KEY_ANIMATION_BACKWARD:
				reverse = true;
			break;

		case KEY_ANIMATION_LINEAR:
				if( pos < currentPosition ) {
					reverse = true;
					a = pos;
					b = currentPosition;
				}
			break;

		case KEY_ANIMATION_CIRCULAR: // QC:U
				if( pos < currentPosition ) {
					a = pos;
					b = currentPosition;
					reverse = true;
				}

				if( (2*( b - a )) > duration ) {
					reverse = !reverse;
					pos = a;
					a = b;
					b = pos;
				}
			break;
	}
	setLoopType(PLAYBACK_ONCE);
	setPlaybackBoundaries(a, b);
}

Renderable* KeyAnimation::getFrameByKey(const char* keyName) {
	assert(keyName);
	assert(*keyName);

	return getFrame(keys[keyName]/period);
}

void KeyAnimation::setPositionByKey(const char* keyName) {
	assert(keyName != NULL);
	assert(keys.find(keyName) != keys.end());

	setPlaybackPosition( keys[keyName] );
	setPlaybackBoundaries(currentPosition, currentPosition);
}

void KeyAnimation::setPlaybackMode(int mode) {
	this->mode = mode;
}

void KeyAnimation::addKey(const char* keyName, int pos) {
	assert(keyName != NULL);

	keys[AOESTRDUP(keyName)] = pos;
}

void KeyAnimation::setPlaybackPosition(int newPosition) {
	setDefaultBoundaries();
	Animation::setPlaybackPosition(newPosition);
}

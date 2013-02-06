#include "audible.h"
#include <math.h>
#include "string.h"
#include "vfs_file.h"

#define HEADER audible
#define HELPER scriptable_object_cpp
#include "include_helper.h"

// QC:?
void Audible::setObjectAudioEffect(AudioEffect* effects) {
	// Handle NULL effects

	if(objectAudioEffect == NULL && effects != NULL) {
		objectAudioEffect = new AudioEffect;
	}
	if(objectAudioEffect != NULL && effects == NULL) {
		delete objectAudioEffect;
		objectAudioEffect = NULL;
	}

	// Set the new effects, if any.
	if(effects != NULL && objectAudioEffect != effects) {
		memcpy(objectAudioEffect, effects, sizeof(AudioEffect));
	}
}

// QC:?
State* Audible::getState() {
	return attachedState;
}

// QC:P
Audible::~Audible() {
	if(objectAudioEffect) {
		delete objectAudioEffect;
		objectAudioEffect = NULL;
	}
}

// QC:?
void Audible::commit(int parentVolume, AudioEffect* effects) {
	AudioEffect* finalAudioEffect;
	int finalVolume;
	computeFinalAttributes(parentVolume, effects, &finalVolume, &finalAudioEffect);

	commitSounds(finalVolume, finalAudioEffect);
}

// QC:?
void Audible::computeFinalAttributes(int parentVolume, AudioEffect* parentEffect, int* finalVolume,
                                     AudioEffect** finalAudioEffectsPtr)
{
	if(finalAudioEffectsPtr != NULL)
		*finalAudioEffectsPtr = finalAudioEffects;

	// Special cases when effects are NULL
	if(!parentEffect) {
		if(objectAudioEffect != NULL) {
			memcpy(finalAudioEffects, objectAudioEffect, sizeof(AudioEffect));
		} else {
			if(finalAudioEffectsPtr)
				*finalAudioEffectsPtr = NULL;
		}
		return;
	}
	if(!objectAudioEffect) {
		memcpy(finalAudioEffects, parentEffect, sizeof(AudioEffect));
		return;
	}

	// Merge effects
	(*finalVolume) = parentVolume * volume / MAX_VOLUME;
}

void Audible::setFadeOutTime(unsigned int fadeOutTime) {
	if(objectAudioEffect == NULL) {
		setNeutralEffect();
	}

	objectAudioEffect->fadeOutTime = fadeOutTime;
}

void Audible::setFadeInTime(unsigned int fadeInTime) {
	if(objectAudioEffect == NULL) {
		setNeutralEffect();
	}

	objectAudioEffect->fadeInTime = fadeInTime;
}

void Audible::setNeutralEffect() {
	if(objectAudioEffect == NULL) {
		objectAudioEffect = new AudioEffect;
	}

	setNeutralEffect(objectAudioEffect);
}

// QC:
void Audible::setNeutralEffect(AudioEffect *effect) {
	effect->fadeInTime = 0;
	effect->currentFadeInTime = 0;
	effect->fadeOutTime = 0;
	effect->currentFadeOutTime = 0;
}

VFS::File* Audible::source() {
	VFS::File* s;
	if((s = ScriptableObject::source()))
		return s;
	return Entity::source();
}

#ifndef AUDIBLE_H
#define AUDIBLE_H

class State;

#include "entity.h"
#include "audio_effect.h"
#include "context.h"

/** An object that can be audible.
 */
class Audible : virtual public ScriptableObject, virtual public Entity {

private:
	void setNeutralEffect();

	// Temporary objects
	AudioEffect* finalAudioEffects;

protected:
	AudioEffect* objectAudioEffect;
	int volume; // 0 -> MAX_VOLUME (10000)

	/** 
	 *
	 */
	virtual void commitSounds(int finalVolume, AudioEffect* finalEffect) = 0;

public:
#define HEADER audible
#define HELPER scriptable_object_h
#include "include_helper.h"
	Audible() { // QC:S
		objectAudioEffect = NULL;
	}

	Audible(State* newAttachedState) : Entity(newAttachedState) { // QC:?
		assert(newAttachedState != NULL);
		objectAudioEffect = NULL;
		attachedState = newAttachedState;
	}

	virtual ~Audible();

	/** Commit this object into the hardware sound system.
	 */
	virtual void commit(int volume, AudioEffect* effects);

	/** Update the status of this object after some time passed.
	 * This function is called by the engine to update the status of this object.
	 * @param elapsedTime the number of milliseconds between this call and the previous one. It can be 0 if the time is unknown.
	 */
	virtual void updateObject(dur elapsedTime) = 0;

	/** Load the data of the audible object.
	 * Used for preloading
	 */
	virtual void loadSounds() = 0;

	/** Unload the data of the audible object.
	 * Used for unloading
	 */
	virtual void unloadSounds() = 0;

	/** Get the current effects linked to this object.
	 * @return the current effects applied to this object.
	 */
	virtual AudioEffect* getObjectAudioEffect() {
		return objectAudioEffect;
	}

	/** Set the audio effect linked to this object.
	 * @param effects the structure holding information about effects to apply to this object. To disable all effects, set it to NULL.
	 */
	virtual void setObjectAudioEffect(AudioEffect* effects);

	/** Return the object's owner.
	 * @return the State which owns this object.
	 */
	virtual State* getState();

	/** Change this object's owner.
	 * @param newState the new owner of this object.
	 */
	void attachToState(State* newState) {
		assert(newState != NULL);
		attachedState = newState;
	}

	/** Set the current volume level of this object.
	 * @param volume the new volume value.
	 */
	void setVolume(int newVolume) {
		volume = newVolume;
	}

	/** Get the current volume level of this object.
	 * @return the volume value.
	 */
	int getVolume() {
		return volume;
	}

	/** Set the fade out time of this object.
	 * @param fadeOutTime the time value for the fade out.
	 */
	void setFadeOutTime(unsigned int fadeOutTime);

	/** Set the fade out time of this object.
	 * @param fadeOutTime the time value for the fade out.
	 */
	void setFadeInTime(unsigned int fadeOutTime);

	/** Set the AudioEffect to neutral.
	 * @param pointer to your AudioEffect.
	 */
	static void setNeutralEffect(AudioEffect *effect);

	/** Compute final attributes of an object given parent's informations.
	 *
	 * @param parentEffect the effects applied to the parent.
	 * @param finalEffect the structure that will store effects to apply to the object.
	 */
	virtual void computeFinalAttributes(int parentVolume, AudioEffect* parentEffect, int* finalVolume, AudioEffect** finalEffect);

	/* AOEObject */

	virtual VFS::File* source();
};
#endif /* AUDIBLE_H */

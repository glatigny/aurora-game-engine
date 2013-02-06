#ifndef AUDIO_EVENT_H
#define AUDIO_EVENT_H

class AudioInstance;

#include "entity.h"
#include "signalable.h"
#include "resource_manager.h"
#include "vfs.h"

typedef ResourceManager<AudioInstance> AudioInstanceResourceManager;

class AudioEvent : virtual public Entity, public Signalable, private AudioInstanceResourceManager {
private:

	virtual bool resLoad(AudioInstance* &returnValue, VFS::File* file);
	virtual void resUnload(AudioInstance* t);

public:
#define HEADER audio_event
#define HELPER scriptable_object_h
#include "include_helper.h"

	AudioEvent() {}
	AudioEvent(State* attachedState) : Entity(attachedState) {}

	/** Construct an audio event.
	 * An audio event is an object linked to a state which produces sound when triggered.
	 * @param attachedState the state linked to this event.
	 * @param filename the file holding audio data to play.
	 */
	AudioEvent(State* attachedState, VFS::File* filename);

	virtual ~AudioEvent() {
		freeResource();
	}

	/* Signalable */
	virtual int signal(ScriptableObject* other, SignalId signalId, int parameter);

	/* AOEObject */
	virtual VFS::File* source() {
		return AudioInstanceResourceManager::source();
	}

};

#endif /* AUDIO_EVENT_H */

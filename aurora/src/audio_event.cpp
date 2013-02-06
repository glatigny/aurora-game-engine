#include "audio_event.h"
#include "hardware_engine.h"

#define HEADER audio_event
#define HELPER scriptable_object_cpp
#include "include_helper.h"

AudioEvent::AudioEvent(State* attachedState, VFS::File* filename) : Entity(attachedState), AudioInstanceResourceManager(attachedState, filename) {}

bool AudioEvent::resLoad(AudioInstance* &returnValue, VFS::File* file) {
	returnValue = engine()->hardware()->instantiateSound(file, AUDIO_FILE_PRELOAD, 10000, 0, false, SOUND_LOOP_ONESHOT, NULL);
	return returnValue != NULL;
}

void AudioEvent::resUnload(AudioInstance* t) {
	engine()->hardware()->freeAudioInstance(t);
}

int AudioEvent::signal(ScriptableObject* other, SignalId signalId, int parameter) {
	if(signalId == PLAY) {
		AudioInstance* soundInstance;
		AudioInstanceResourceManager::getResource(soundInstance);

		if(soundInstance) {
			engine()->hardware()->pause(soundInstance);
			engine()->hardware()->rewind(soundInstance);
			engine()->hardware()->play(soundInstance);
		}

		return 1;
	}

	return 0;
}
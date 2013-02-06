#include "sound_engine.h"

// QC:?
SoundEngine::~SoundEngine() {
	// Release sounds
	for(InstanceAssociation::iterator t = instanciatedSounds.begin(); t != instanciatedSounds.end(); t++) {
		for(std::list<AudioInstance*>::iterator u = (*t).second.begin(); u != (*t).second.end(); u++) {
			myFreeAudioInstance((*u));
		}
	}
	instanciatedSounds.clear();

	AudioPreloadBuffer* tmp;
	for(std::map<char*, AudioPreloadBuffer*, strCmp>::iterator t = loadedSounds.begin(); t != loadedSounds.end(); t++) {
		tmp = ((*t).second);
		//FIXME : necessary ?		if( tmp != NULL )
		//			delete tmp;
		AOEFREE((void*) ((*t).first));
		myFreeAudioPreloadBuffer((*t).second);
	}
	loadedSounds.clear();

	for(std::list<AudioInstance*>::iterator t = loadedStreams.begin(); t != loadedStreams.end(); t++) {
		myFreeAudioInstance((*t));
	}
	loadedStreams.clear();
}

// QC:?
AudioInstance* SoundEngine::instantiateSound(VFS::File* file, int audioAccessMode, int initialVolume, int priority,
                                             bool positioned, int loop, Translatable* position)
{
	assert(file);
	AudioInstance* instance = NULL;

	if(audioAccessMode == AUDIO_FILE_PRELOAD) {
		std::map<char*, AudioPreloadBuffer*, strCmp>::iterator i = loadedSounds.find((char*) file->name());
		if(i != loadedSounds.end()) {
			instance = myInstantiateSound(i->second, audioAccessMode, initialVolume, priority, positioned, loop, position);
		} else {
			instance = myInstantiateSound(file, audioAccessMode, initialVolume, priority, positioned, loop, position);
			assert(instance);
			if(instance->buffer) {
				loadedSounds[AOESTRDUP(file->name())] = instance->buffer;
			}
		}
		instanciatedSounds[instance->buffer].push_back(instance);
	} else {
		instance = myInstantiateSound(file, audioAccessMode, initialVolume, priority, positioned, loop, position);
		loadedStreams.insert(loadedStreams.end(), instance);
	}

	assert(instance);
	return instance;
}

// QC:?
void SoundEngine::freeAudioInstance(AudioInstance* instance) {
	AudioPreloadBuffer *audio = instance->buffer;
	InstanceAssociation::iterator i = instanciatedSounds.find(audio);
	if(i != instanciatedSounds.end()) {
		i->second.remove(instance);
		if(i->second.size() == 0) {
			AOEFREE(i->first);
			myFreeAudioInstance(instance);
			instanciatedSounds.erase(i);
		}
	} else {
		myFreeAudioInstance(instance);
	}
}

// QC:W
bool SoundEngine::myFreeAudioInstance(AudioInstance* instance) {
	// Responsible for memory leaks !
	return false;
}

// QC:W
bool SoundEngine::myFreeAudioPreloadBuffer(AudioPreloadBuffer* audioBuffer) {
	// Responsible for memory leaks !
	return false;
}


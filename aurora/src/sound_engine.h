#ifndef SOUND_ENGINE_H
#define SOUND_ENGINE_H

#include "translatable.h"
#include "vfs.h"

#include <map>
#include <list>

#define AUDIO_FILE_PRELOAD 0
#define AUDIO_FILE_STREAM 1

#define SOUND_LOOP_ONESHOT 0
#define SOUND_LOOP_FOREVER 1

typedef void AudioPreloadBuffer;

class HardwareEngine;
namespace VFS {
	class PCMSource;
}

class AudioInstance {
public:
	AudioPreloadBuffer *buffer;
};

class SoundEngineInterface {
public:
	virtual void init(HardwareEngine* hw) {}
	virtual ~SoundEngineInterface() = 0;

	/** Instantiate a sound from a file.
	 * This function is an overload loading the file immediately.
	 *
	 * @param filename the name of the file to open.
	 * @param audioAccessMode tells whether the sound must be preloaded, loaded on access or streamed.
	 * @param initialVolume the volume of the sound.
	 * @param priority the priority of the sound. Higher priority sounds cannot be interrupted by lower priority ones.
	 * @param positioned true to make the sound positioned, false to leave it centered. Please note that stereo sounds cannot be positioned.
	 * @param loop determines the loop mode of the sound. Currently, only one shot (0) and normal loop (1) are defined.
	 * @param position the translatable giving the position of the sound.
	 * @return an audio instance structure.
	 */
	virtual AudioInstance* instantiateSound(VFS::File* file, int audioAccessMode, int initialVolume, int priority, bool positioned,
	                                        int loop, Translatable* position) = 0;

	/** Free an audio instance.
	 */
	virtual void freeAudioInstance(AudioInstance* instance) = 0;

	/** Starts playing an instance.
	 */
	virtual bool play(AudioInstance* sound) = 0;

	/** Indicate if an instance is playing.
	 */
	virtual bool isPlaying(AudioInstance* sound) = 0;

	/** Pauses an instance.
	 */
	virtual void pause(AudioInstance* sound) = 0;

	/** Rewinds an instance.
	 */
	virtual void rewind(AudioInstance* sound) = 0;
};

class SoundEngine: public SoundEngineInterface {
protected:
	// Map comparator
	struct strCmp {
		bool operator()(const char* s1, const char* s2) const {
			return strcmp(s1, s2) < 0;
		}
	};

	typedef std::map<AudioPreloadBuffer*, std::list<AudioInstance*> > InstanceAssociation;

	std::map<char*, AudioPreloadBuffer*, strCmp> loadedSounds;
	std::list<AudioInstance*> loadedStreams;
	InstanceAssociation instanciatedSounds;

	virtual AudioPreloadBuffer* myLoadAudioPreloadBuffer(VFS::File* file, int audioAccessMode) = 0;
	virtual bool myFreeAudioPreloadBuffer(AudioPreloadBuffer* audioBuffer); // Should be pure virtual, but called from destructor.

	virtual AudioInstance* myInstantiateSound(VFS::File* file, int audioAccessMode, int initialVolume, int priority, bool positioned,
	                                          int loop, Translatable* position) = 0;
	virtual AudioInstance* myInstantiateSound(AudioPreloadBuffer* buffer, int audioAccessMode, int initialVolume, int priority,
	                                          bool positioned, int loop, Translatable* position) = 0;
	virtual bool myFreeAudioInstance(AudioInstance* instance); // Should be pure virtual, but called from destructor.

public:
	virtual ~SoundEngine();

	virtual AudioInstance* instantiateSound(VFS::File* file, int audioAccessMode, int initialVolume, int priority, bool positioned,
	                                        int loop, Translatable* position);

	virtual void freeAudioInstance(AudioInstance* instance);

};

#endif /* SOUND_ENGINE_H */

#ifndef OPEN_AL_H
#define OPEN_AL_H

#include "../../sound_engine.h"
#include "../../vfs.h"
#include "../../vfs/pcm_source.h"
#include <AL/al.h>
#include <AL/alc.h>
#include "pspec/thread.h"

#define OPENAL_STREAM_BUFFER_SIZE 65536
#define OPENAL_STREAM_BUFFER_COUNT 3


class OpenAlInstance;
class OpenAlStreamingData {
public:
	enum {
		NoInstance,
		Stop,
		Play,
		Pause,
		Fadeout
	} control;
	SharedMutex streamMutex;
	OpenAlInstance* instance;

	OpenAlStreamingData(OpenAlInstance* sourceInstance);
};

class OpenAlInstance: public AudioInstance {
public:
	ALuint source;
	ALenum format;
	VFS::PCMSource* pcmSource;
	OpenAlStreamingData* stream;
	SharedMutex streamMutex;
	int loop;
};

class OpenAlSoundEngine: public SoundEngine {
private:
	OpenAlSoundEngine() {
		assert(false);
	}
	OpenAlSoundEngine(ALCdevice *alDevice, ALCcontext *alContext);

	virtual AudioPreloadBuffer* myLoadAudioPreloadBuffer(VFS::File* file, int audioAccessMode);
	virtual bool myFreeAudioPreloadBuffer(AudioPreloadBuffer* audioBuffer);

	virtual AudioInstance* myInstantiateSound(VFS::File* file, int audioAccessMode, int initialVolume, int priority, bool positioned,
	                                          int loop, Translatable* position);
	virtual AudioInstance* myInstantiateSound(AudioPreloadBuffer* buffer, int audioAccessMode, int initialVolume, int priority,
	                                          bool positioned, int loop, Translatable* position);
	virtual bool myFreeAudioInstance(AudioInstance* instance);

public:
	virtual ~OpenAlSoundEngine();

	static OpenAlSoundEngine* open();

	virtual AudioInstance* instantiateSound(VFS::File* file, int audioAccessMode, int initialVolume, int priority, bool positioned,
	                                        int loop, Translatable* position) { return myInstantiateSound(file, audioAccessMode, initialVolume, priority, positioned, loop, position); }
	
	virtual bool play(AudioInstance* sound);
	virtual bool isPlaying(AudioInstance* sound);
	virtual void pause(AudioInstance* sound);
	virtual void rewind(AudioInstance* sound);

	static Mutex hw;
};

#endif /* OPEN_AL_H */

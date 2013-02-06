#include "open_al.h"
#include "../thread.h"
#include <list>

Mutex OpenAlSoundEngine::hw;

static inline void alCheck();
static void streamingTask(void* streamInstance);
static bool streamBuffer(ALuint buffer, VFS::PCMSource* pcmSource, ALuint format, ALuint source);

// QC:G
OpenAlSoundEngine* OpenAlSoundEngine::open() {
	ALCcontext *alContext = NULL;
	ALCdevice *alDevice = NULL;

#ifndef NDEBUG
	std::list<const char*> availableDevices;

	// Récupération des devices disponibles
	const ALCchar* deviceList = alcGetString(NULL, ALC_DEVICE_SPECIFIER);

	// Extraction des devices contenus dans la chaîne renvoyée
	syslog("Available OpenAL devices:");
	while(deviceList && *deviceList) {
		syslog(" %s", deviceList);
		availableDevices.push_back(deviceList);
		deviceList += strlen(deviceList) + 1;
	}
	syslog("End of OpenAL device list.");
#endif

	hw.p();

	alDevice = alcOpenDevice(NULL);
	if(!alDevice) {
		return NULL;
	}

	alContext = alcCreateContext(alDevice, NULL);
	if(!alContext) {
		alcCloseDevice(alDevice);
		return NULL;
	}

	hw.v();

	return new OpenAlSoundEngine(alDevice, alContext);
}

// QC:G
OpenAlSoundEngine::OpenAlSoundEngine(ALCdevice *alDevice, ALCcontext *alContext) {
	syslog("\nOpened %s Device\n", alcGetString(alDevice, ALC_DEVICE_SPECIFIER));
	hw.p();
	alcMakeContextCurrent(alContext);
	hw.v();
}

// QC:G
OpenAlSoundEngine::~OpenAlSoundEngine() {
	hw.p();
	ALCcontext *alContext;
	ALCdevice *alDevice;

	alContext = alcGetCurrentContext();
	alDevice = alcGetContextsDevice(alContext);

	alcMakeContextCurrent(NULL);
	alcDestroyContext(alContext);
	alcCloseDevice(alDevice);
	hw.v();
}

// QC:?
AudioPreloadBuffer* OpenAlSoundEngine::myLoadAudioPreloadBuffer(VFS::File* file, int audioAccessMode) {
	AudioPreloadBuffer* buffer = new ALuint;
	VFS::PCMSource* s = VFS::openPCM(file);
	char* data;
	size_t size;
	xassert(s, "Cannot load sound %s.", file->name());
	s->load(data, size, 0);

	hw.p();
	alGenBuffers(1, (ALuint*) buffer);
	alBufferData(*((ALuint*) buffer), (s->getChannels() == 2) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16, data, size, s->getFrequency());
	hw.v();

	return buffer;
}

// QC:?
bool OpenAlSoundEngine::myFreeAudioPreloadBuffer(AudioPreloadBuffer* audioBuffer) {
	assert(audioBuffer);
	
	hw.p();
	alDeleteBuffers(1, (ALuint*) audioBuffer);
	hw.v();

	delete (ALuint*)audioBuffer;
	return true;
}

// QC:?
AudioInstance* OpenAlSoundEngine::myInstantiateSound(VFS::File* file, int audioAccessMode, int initialVolume, int priority,
                                                     bool positioned, int loop, Translatable* position)
{
	if(audioAccessMode == AUDIO_FILE_PRELOAD) {
		return myInstantiateSound(myLoadAudioPreloadBuffer(file, AUDIO_FILE_PRELOAD), AUDIO_FILE_PRELOAD, initialVolume, priority,
		                          positioned, loop, position);
	}

#ifdef THREADS_SUPPORT
	xerror(audioAccessMode == AUDIO_FILE_STREAM, "Unsupported audio access mode.");

	OpenAlInstance* instance = new OpenAlInstance();
	instance->stream = new OpenAlStreamingData(instance);

	VFS::PCMSource* PCMfile = VFS::openPCM(file);
	xassert(PCMfile, "Cannot load sound %s for buffering.", file->name());

	instance->format = AL_FORMAT_STEREO16;

	hw.p();
	alGenSources(1, &instance->source);
	alSource3f(instance->source, AL_POSITION, 0.0, 0.0, 0.0);
	alSource3f(instance->source, AL_VELOCITY, 0.0, 0.0, 0.0);
	alSource3f(instance->source, AL_DIRECTION, 0.0, 0.0, 0.0);
	alSourcef(instance->source, AL_ROLLOFF_FACTOR, 0.0);
	alSourcei(instance->source, AL_SOURCE_RELATIVE, AL_TRUE );
	hw.v();

	instance->buffer = NULL;
	instance->pcmSource = PCMfile;
	instance->loop = loop;
	instance->stream->instance = instance;
	instance->stream->control = OpenAlStreamingData::Stop;

	return instance;
#else
	xwarn(false, "No threads support : cannot play streaming media with OpenAL.");
	return NULL;
#endif
}

// QC:?
AudioInstance* OpenAlSoundEngine::myInstantiateSound(AudioPreloadBuffer* buffer, int audioAccessMode, int initialVolume,
                                                     int priority, bool positioned, int loop, Translatable* position)
{
	OpenAlInstance* instance = new OpenAlInstance();

	instance->stream = NULL;

	assert(audioAccessMode == AUDIO_FILE_PRELOAD);

	instance->buffer = buffer;
	instance->loop = loop;

	hw.p();
	alGenSources(1, &(instance->source));
	alSourcei(instance->source, AL_BUFFER, *(ALint*) (instance->buffer));
	alSourcef(instance->source, AL_PITCH, 1.0f);
	alSourcef(instance->source, AL_GAIN, 1.0f);
	alSource3f(instance->source, AL_POSITION, 0.0, 0.0, 0.0);
	alSource3f(instance->source, AL_VELOCITY, 0.0, 0.0, 0.0);
	alSource3f(instance->source, AL_DIRECTION, 0.0, 0.0, 0.0);
	alSourcei(instance->source, AL_LOOPING, loop);
	alSourcei(instance->source, AL_SOURCE_RELATIVE, AL_TRUE );
	hw.v();

	return instance;
}

// QC:?
bool OpenAlSoundEngine::myFreeAudioInstance(AudioInstance* sound) {
	assert(sound);
	OpenAlInstance* ai = ((OpenAlInstance*) sound);

	int queued;

	ai->streamMutex.p();
	if(ai->stream) {
		assert(ai->streamMutex == ai->stream->streamMutex);
		assert(ai->stream->instance == sound);

		switch(ai->stream->control) {
			case OpenAlStreamingData::Play:
				// Fade out and let the thread terminate itself.
				ai->stream->control = OpenAlStreamingData::Fadeout;
				ai->stream->instance = NULL;
				ai->streamMutex.v();
				delete sound;
				return true;

			case OpenAlStreamingData::Pause:
				// Fade out and let the thread terminate itself.
				ai->stream->control = OpenAlStreamingData::Stop;
			default:
				ai->stream->instance = NULL;
				ai->streamMutex.v();
				delete sound;
				return true;
		}
	}
	ai->streamMutex.v();

	if(ai->buffer) {
		hw.p();

		alSourceStop(ai->source);

		alGetSourcei(ai->source, AL_BUFFERS_QUEUED, &queued);
		while(queued--) {
			ALuint buffer;
			alSourceUnqueueBuffers(ai->source, 1, &buffer);
		}

		alDeleteSources(1, &ai->source);
		alCheck();

		hw.v();
	}

	delete sound;
	return true;
}

// QC:?
bool OpenAlSoundEngine::isPlaying(AudioInstance* sound) {
	assert(sound);
	OpenAlInstance* ai = ((OpenAlInstance*) sound);

	ai->streamMutex.p();
	if(ai->stream) {
		bool playing = (ai->stream->control == OpenAlStreamingData::Play);
	} 
	ai->streamMutex.v();
	
	if(ai->buffer) {
		ALint state;

		hw.p();
		alGetSourcei(ai->source, AL_SOURCE_STATE, &state);
		hw.v();

		return state == AL_PLAYING;
	}

	return false;
}

// QC:?
bool OpenAlSoundEngine::play(AudioInstance* sound) {
	assert(sound);
	OpenAlInstance* ai = ((OpenAlInstance*) sound);

	if(!isPlaying(sound)) {
		if(ai->buffer) {
			hw.p();
			alSourcePlay(ai->source);
			hw.v();
		} else {
#ifdef THREADS_SUPPORT
			// Thread Creation
			ai->streamMutex.p();
			if(ai->stream)
				switch(ai->stream->control) {
					case OpenAlStreamingData::Pause:
						hw.p();
						alSourcePlay(ai->source);
						hw.v();
						ai->stream->control = OpenAlStreamingData::Play;
						break;

					case OpenAlStreamingData::Stop:
					case OpenAlStreamingData::NoInstance:
						ai->stream->control = OpenAlStreamingData::Play;
						createThread(streamingTask, ai->stream, 2);
						break;

					case OpenAlStreamingData::Play:
					case OpenAlStreamingData::Fadeout:
					default:
						break;
				}
			ai->streamMutex.v();
#else
			xassert(false, "No threads support : cannot play in streaming mode.");
#endif
		}
	}
	return true;
}

// QC:U
void OpenAlSoundEngine::pause(AudioInstance* sound) {
	assert(sound);
	OpenAlInstance* ai = ((OpenAlInstance*) sound);
	if(ai->buffer) {
		hw.p();
		alSourcePause(ai->source);
		hw.v();
	} else {
		ai->streamMutex.p();
		if(ai->stream) {
			ai->stream->control = OpenAlStreamingData::Pause;
			hw.p();
			alSourcePause(ai->source);
			hw.v();
		}
		ai->streamMutex.v();
	}
}

// QC:?
void OpenAlSoundEngine::rewind(AudioInstance* sound) {
	assert(sound);
	OpenAlInstance* ai = ((OpenAlInstance*) sound);

	if(ai->buffer) {
		hw.p();
		alSourceRewind(ai->source);
		hw.v();
	} else {
		ai->streamMutex.p();
		if(ai->stream) {
			ai->pcmSource->seek(0, SEEK_SET);
		}
		ai->streamMutex.v();
	}
}

static void fillBuffers(OpenAlStreamingData* stream, ALuint source, ALuint format, ALuint buffers[], VFS::PCMSource* pcmSource, int loop) {
	assert(stream);
	int processed;
	alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);

	while(!processed && stream->control == OpenAlStreamingData::Play) {
		stream->streamMutex.v();
		waitMillis(16);
		stream->streamMutex.p();

		alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
	}

	while(processed--) {
		ALuint buffer;

		alSourceUnqueueBuffers(source, 1, &buffer);
		alCheck();

		if(!streamBuffer(buffer, pcmSource, format, source)) {
			if( loop > 0 ) {
				pcmSource->seek(0, SEEK_SET);
				streamBuffer(buffer, pcmSource, format, source);
			} else {
				stream->control = OpenAlStreamingData::Stop;
				break;
			}
		}
	}

	// Check for buffer underruns
	ALint state;
	alGetSourcei(source, AL_SOURCE_STATE, &state);
	if(stream->control == OpenAlStreamingData::Play && state != AL_PLAYING && !pcmSource->eof()) {
		alSourcePlay(source);
	}
}

// QC:G
static void streamingTask(void* streamInstance) {
#ifdef WIN32
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
#endif

	OpenAlSoundEngine::hw.p();

	OpenAlStreamingData* stream = (OpenAlStreamingData*) streamInstance;
	assert(stream);
	assert(stream->instance);

	stream->streamMutex.p();

	ALuint buffers[OPENAL_STREAM_BUFFER_COUNT];
	ALuint source = stream->instance->source;
	ALenum format = stream->instance->format;
	int loop = stream->instance->loop;
	VFS::PCMSource* pcmSource = stream->instance->pcmSource;

	alGenBuffers(OPENAL_STREAM_BUFFER_COUNT, buffers);

	// Pre-fill buffer
	for(int i = 0; i < OPENAL_STREAM_BUFFER_COUNT; i++) {
		if(!streamBuffer(buffers[i], pcmSource, format, source)) {
			stream->control = OpenAlStreamingData::Stop;
		}
	}

	alSourcePlay(source);

	int waitTime = (OPENAL_STREAM_BUFFER_SIZE * 1000 / pcmSource->getFrequency()) / pcmSource->getSampleSize() / pcmSource->getChannels();
	waitTime *= OPENAL_STREAM_BUFFER_COUNT - 1;
	waitTime -= 1;
	xerror(waitTime >= 3, "OpenAL stream buffer too small : %d milliseconds.", waitTime);

	// Main loop
	while(stream->control == OpenAlStreamingData::Play || stream->control == OpenAlStreamingData::Pause) {
		xassert(!stream->instance || source == stream->instance->source, "openAL spurious source change.");

		fillBuffers(stream, source, format, buffers, pcmSource, loop);

		stream->streamMutex.v();
		OpenAlSoundEngine::hw.v();
		waitMillis(waitTime);
		OpenAlSoundEngine::hw.p();
		stream->streamMutex.p();

		assert(stream->control == OpenAlStreamingData::Play || stream->control == OpenAlStreamingData::Pause || stream->control == OpenAlStreamingData::Fadeout || stream->control == OpenAlStreamingData::Stop);
	}
	assert(stream->control == OpenAlStreamingData::Fadeout || stream->control == OpenAlStreamingData::Stop);

	// Fade-out (FIXME : fillBuffers may block)
	for(float g=1.0f; g>0.001f && stream->control == OpenAlStreamingData::Fadeout; g*=0.96) {
		ALint state;
		alGetSourcei(source, AL_SOURCE_STATE, &state);
		if(state != AL_PLAYING || pcmSource->eof())
			break;
		alSourcef(source, AL_GAIN, g);

		fillBuffers(stream, source, format, buffers, pcmSource, loop);

		stream->streamMutex.v();
		OpenAlSoundEngine::hw.v();
		waitMillis(6);
		OpenAlSoundEngine::hw.p();
		stream->streamMutex.p();
	}

	if(stream->instance) {
		stream->instance->stream = NULL;
		stream->streamMutex.v();
	}

	int processed;
	alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);

	while(!processed && stream->control == OpenAlStreamingData::Play) {
		if(stream->instance)
			stream->instance->streamMutex.v();
		OpenAlSoundEngine::hw.v();
		waitMillis(16);
		OpenAlSoundEngine::hw.p();
		if(stream->instance)
			stream->instance->streamMutex.p();

		alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
	}

	alSourceStop(source);

	alGetSourcei(source, AL_BUFFERS_QUEUED, &processed);
	while(processed--) {
		ALuint buffer;
		alSourceUnqueueBuffers(source, 1, &buffer);
	}

	alDeleteBuffers(OPENAL_STREAM_BUFFER_COUNT, buffers);
	alCheck();

	alDeleteSources(1, &source);
	alCheck();

	delete pcmSource;
	delete stream;

	OpenAlSoundEngine::hw.v();
}

// QC:P (assert correctement.)
static inline void alCheck() {
#ifndef NDEBUG
	ALenum error = alGetError();
	const char* errorString = "Undefined error";
	switch(error)
	{
			case AL_NO_ERROR:
					errorString = "AL_NO_ERROR";
			break;

			case AL_INVALID_NAME:
					errorString = "AL_INVALID_NAME";
			break;

			case AL_INVALID_ENUM:
					errorString = "AL_INVALID_ENUM";
			break;

			case AL_INVALID_VALUE:
					errorString = "AL_INVALID_VALUE";
			break;

			case AL_INVALID_OPERATION:
					errorString = "AL_INVALID_OPERATION";
			break;

			case AL_OUT_OF_MEMORY:
					errorString = "AL_OUT_OF_MEMORY";
			break;
	}

	xassert(error == AL_NO_ERROR, "OpenAL error. Code=%s", errorString);
#endif
}

// QC:G (encore pas 100% des cas d'erreur gérés. Un effort de step by step à faire)
static bool streamBuffer(ALuint buffer, VFS::PCMSource* pcmSource, ALuint format, ALuint source) {
	short data[OPENAL_STREAM_BUFFER_SIZE];

	size_t result = pcmSource->read(data, OPENAL_STREAM_BUFFER_SIZE);
	if(!result) {
		assert(pcmSource->eof());
		return false;
	}

	alBufferData(buffer, format, data, result, pcmSource->getFrequency());
	alSourceQueueBuffers(source, 1, &buffer);
	alCheck();

	return true;
}


OpenAlStreamingData::OpenAlStreamingData(OpenAlInstance *sourceInstance) : streamMutex(sourceInstance->streamMutex), instance(sourceInstance) {}
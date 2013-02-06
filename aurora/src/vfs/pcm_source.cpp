#include "pcm_source.h"
#ifdef VORBIS_SUPPORT
#include "decoders/vorbis_pcm_source.h"
#endif

namespace VFS {

#ifdef PSPEC_OPEN_PCM
PCMSource* PSPEC_OPEN_PCM(File* sourceFile);
#endif

#ifdef GAMESPEC_OPEN_PCM
PCMSource* GAMESPEC_OPEN_PCM(File* sourceFile);
#endif

PCMSource::~PCMSource() {}

PCMSource* openPCM(File* sourceFile) {
#ifdef PSPEC_OPEN_PCM
	PCMSource* s = PSPEC_OPEN_PCM(sourceFile);
	if(s) {
		return s;
	}
#endif

#ifdef VORBIS_SUPPORT
	if(VorbisPCMSource::handles(sourceFile)) {
		return new VorbisPCMSource(sourceFile);
	}
#endif

#ifdef GAMESPEC_OPEN_PCM
	return GAMESPEC_OPEN_PCM(sourceFile);
#endif

	xwarn(NULL, "Cannot open PCM file %s.", sourceFile->name());
	return NULL;
}

unsigned int PCMSource::getSampleSize() {
	switch(getFormat()) {
	case PCM_FORMAT_8U:
	case PCM_FORMAT_8S:
	return getChannels();

	case PCM_FORMAT_16UBE:
	case PCM_FORMAT_16SBE:
	case PCM_FORMAT_16ULE:
	case PCM_FORMAT_16SLE:
	return getChannels()*2;

	case PCM_FORMAT_32F:
	return getChannels()*4;

	case PCM_FORMAT_IMA4:
	return getChannels()/2;
	}
	return 0;
}

}


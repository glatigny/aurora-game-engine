#ifndef VFS__VORBIS_SOURCE_H
#define VFS__VORBIS_SOURCE_H

#include "../pcm_source.h"
#include <vorbis/vorbisenc.h>
#include <vorbis/vorbisfile.h>

namespace VFS {

class VorbisPCMSource : public PCMSource {
private:
	static ov_callbacks callbacks;
	File* f;
	OggVorbis_File of;
	int loopType;
	bool canSeek;
	int section;
	bool eofReached;

public:
	VorbisPCMSource(File* f);
	virtual ~VorbisPCMSource();

	/* PCMSource */
	virtual VorbisPCMSource* copy() { return new VorbisPCMSource(f); }
	virtual int getFormat() { return PCM_FORMAT_16S; /* TODO */ }
	virtual int setFormat(int newFormat, bool allowConversion = false) { return PCM_FORMAT_16S; /* TODO */ }
	virtual int getFrequency() { return ov_info(&of, -1)->rate; }
	virtual int getChannels() { return ov_info(&of, -1)->channels; }
	virtual bool setLoop(int newLoopType) { loopType = newLoopType; return true; }
	virtual unsigned int getCapabilities();
	virtual size_t read(void* outputBuffer, size_t outputBufferLen);
	virtual unsigned int load(char* &outputBuffer, size_t &outputBufferLen, size_t maxOutputBufferLen);
	virtual bool eof() { return eofReached; }
	virtual size_t seek(size_t position, int whence);
	static bool handles(File* sourceFile);
};

}

#endif /* VFS__VORBIS_SOURCE_H */


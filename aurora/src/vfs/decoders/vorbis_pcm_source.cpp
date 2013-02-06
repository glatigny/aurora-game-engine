#include "vorbis_pcm_source.h"
#include "../posix_file.h"

namespace VFS {

size_t vorbisfile_read(void* ptr, size_t size, size_t nmemb, void* datasource);
int vorbisfile_seek(void* datasource, ogg_int64_t offset, int whence);
int vorbisfile_close(void* datasource);
long vorbisfile_tell(void* datasource);

ov_callbacks VorbisPCMSource::callbacks = { vorbisfile_read, vorbisfile_seek, vorbisfile_close, vorbisfile_tell };

VorbisPCMSource::VorbisPCMSource(File* vf) :
	canSeek(false), section(0), eofReached(false)
{
	f = vf->copy();
	f->switchMode(MODE_READ);
	canSeek = f->canSeek();
	if(ov_open_callbacks(f, &of, NULL, 0, callbacks) < 0) {
		syslog("Cannot open OGG Vorbis file %s.", f->name());
		assert(false);
	}
}

VorbisPCMSource::~VorbisPCMSource() {
	ov_clear(&of);
	if( f )
		delete f;
}

size_t vorbisfile_read(void* ptr, size_t size, size_t nmemb, void* datasource) {
	File* f = (File*) datasource;
	assert(f);
	return f->read(ptr, size * nmemb) / size;
}

int vorbisfile_seek(void* datasource, ogg_int64_t offset, int whence) {
	File* f = (File*) datasource;
	assert(f);

	if(f->canSeek(offset, whence) && f->seek(offset, whence)) {
		return 0;
	}

	return -1;
}

int vorbisfile_close(void* datasource) {
	return 0;
}

long vorbisfile_tell(void* datasource) {
	File* f = (File*) datasource;
	assert(f);
	return f->tell();
}

// QC:A
unsigned int VorbisPCMSource::getCapabilities() {
	return canSeek ? (PCM_SOURCE_CAN_REWIND | PCM_SOURCE_CAN_SEEK/*|PCM_SOURCE_CAN_LOOP*/) : PCM_SOURCE_CAN_GROW;
}

// QC:G
size_t VorbisPCMSource::read(void* outputBuffer, size_t outputBufferLen) {
	// TODO : implement loop
	char* buf = (char*) outputBuffer;
	size_t totalBytes = 0;
	while(outputBufferLen && !eofReached) {
		long bytes = ov_read(&of, buf, outputBufferLen, 0, 2, 1, &section);
		if(bytes == OV_HOLE
		|| bytes == OV_EBADLINK
		|| bytes == OV_EINVAL) {
			xwarn(false, "Problem while reading %s.", f->name());
			return 0;
		} else if(bytes == 0) {
			eofReached = true;
			return 0;
		}
		totalBytes += bytes;
		outputBufferLen -= bytes;
		buf += bytes;
	}
	return totalBytes;
}

// QC:?
unsigned int VorbisPCMSource::load(char* &outputBuffer, size_t &outputBufferLen, size_t maxOutputBufferLen) {
	ogg_int64_t samples = ov_pcm_total(&of, -1);

	if(getCapabilities() & PCM_SOURCE_CAN_SEEK)
		seek(0, SEEK_SET);

	outputBufferLen = samples * getChannels() * 2;
	if(maxOutputBufferLen && outputBufferLen > maxOutputBufferLen) {
		outputBufferLen = maxOutputBufferLen;
	}
	outputBuffer = (char*) AOEMALLOC(outputBufferLen);
	outputBufferLen = read(outputBuffer, outputBufferLen);

	if(eof()) {
		return 0;
	} else {
		if(outputBufferLen == 0) {
			return 2;
		} else {
			return 1;
		}
	}
}

// QC:U
size_t VorbisPCMSource::seek(size_t position, int whence) {
	assert(canSeek);
	xerror(whence == SEEK_SET, "VorbisPCMSource::seek : whence parameter not supported.");
	if(!ov_pcm_seek(&of, position)) {
		eofReached = false;
		return position;
	}

	xerror(false, "VorbisPCMSource::seek : Could not seek at position %d in %s.", position, f->name());
	return 0;
}

// QC:A
bool VorbisPCMSource::handles(File* sourceFile) {
	return strstr(sourceFile->name(), ".ogg") != NULL; /* XXX : better detection needed */
}

}

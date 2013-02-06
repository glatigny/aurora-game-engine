#include "buffer_image_source.h"

namespace VFS {

// QC:?
BufferImageSource::BufferImageSource(char* newBuffer, int newFormat, int newWidth, int newHeight) : ImageSource(NULL), buffer(newBuffer), format(newFormat), width(newWidth), height(newHeight) {}

// QC:?
BufferImageSource::~BufferImageSource() {
	AOEFREE(buffer);
}

// QC:?
BufferImageSource* BufferImageSource::copy() {
	size_t bufferSize = ImageSource::getPitch(format, width) * height;
	char* newBuffer = (char*)AOEMALLOC(bufferSize);
	memcpy(newBuffer, buffer, bufferSize);
	return new BufferImageSource(newBuffer, format, width, height);
}

// QC:P
int BufferImageSource::getPreferredFormat() {
	return format;
}

// QC:?
void BufferImageSource::load(char* &dstBuffer, Rect &dstBufferSize, size_t& dstBufferLen, size_t& dstPitch, int &dstFormat, Rect &dstImageSize, bool dstForcePOT, bool dstAllowConversion) {
	xassert(!dstForcePOT || (!(width & (width-1)) && !(height & (height-1))), "BufferSource does not support POT conversion.");
	xassert(!dstAllowConversion, "BufferSource does not support conversions.");

	dstBuffer = buffer;
	dstPitch = ImageSource::getPitch(format, width);
	dstFormat = format;
	dstBufferLen = dstPitch * height;
	dstImageSize = Rect(0, 0, width, height);
	dstBufferSize = Rect(0, 0, width, height);
}

}

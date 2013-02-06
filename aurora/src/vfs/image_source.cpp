#include "image_source.h"
#ifdef PNG_SUPPORT
#include "decoders/png_image_source.h"
#endif

#include "buffer_image_source.h"

namespace VFS {

ImageSource* openImage(File* sourceFile) {
#ifdef PNG_SUPPORT
	if(PNGImageSource::handles(sourceFile)) {
		return new PNGImageSource(sourceFile);
	}
#endif

	return NULL;
}

ImageSource* createImage(char* buffer, int format, int width, int height) {
	return new BufferImageSource(buffer, format, width, height);
}

bool saveImage(File* destFile, ImageSource* source) {
	char* buffer;
	Rect bufferSize;
	size_t bufferLen;
	size_t pitch;
	int format;
	Rect imageSize;

	source->load(buffer, bufferSize, bufferLen, pitch, format, imageSize);

#ifdef PNG_SAVE_SUPPORT
	if(PNGImageSource::handlesWriting(destFile, format)) {
		return PNGImageSource::save(destFile, buffer, format, bufferSize.getW(), bufferSize.getH());
	} else
#endif
	{
		xerror(false, "Cannot save image %s : unhandled format %d", destFile->name(), format);
		return false;
	}

	AOEFREE(buffer);
	return true;
}

ImageSource::ImageSource(File* sourceFile) : file(sourceFile->copy()) {}

ImageSource::~ImageSource() {
	xassert(file != NULL, "Cannot find file descriptor.");
	delete(file);
}

bool ImageSource::convert(const char* inputBuffer, size_t inputBufferSize, int inputFormat, char* &outputBuffer, size_t &outputBufferSize, int outputFormat) {
	switch(inputFormat) {
		case IMAGE_R8G8B8A8: {
			switch(outputFormat) {
				case IMAGE_A8R8G8B8: {
					outputBufferSize = inputBufferSize;
					outputBuffer = (char*)AOEMALLOC(outputBufferSize);
					uint32_t* ib = (uint32_t*)inputBuffer;
					uint32_t* ob = (uint32_t*)outputBuffer;
					for(size_t i=0; i < inputBufferSize/4; i++) {
						uint32_t v = ib[i];
#ifdef BIG_ENDIAN
						v = v << 24 | v >> 8;
#else
						v = v >> 24 | v << 8;
#endif
						ob[i] = v;
					}
				} return true;
			}
		} break;
		case IMAGE_A8R8G8B8: {
			switch(outputFormat) {
				case IMAGE_R8G8B8A8: {
					outputBufferSize = inputBufferSize;
					outputBuffer = (char*)AOEMALLOC(outputBufferSize);
					uint32_t* ib = (uint32_t*)inputBuffer;
					uint32_t* ob = (uint32_t*)outputBuffer;
					for(size_t i=0; i < inputBufferSize/4; i++) {
						uint32_t v = ib[i];
#ifdef BIG_ENDIAN
						v = v >> 24 | v << 8;
#else
						v = v << 24 | v >> 8;
#endif
						ob[i] = v;
					}
				} return true;
			}
		} break;
		case IMAGE_R8G8B8: {
			switch(outputFormat) {
				case IMAGE_R8G8B8A8: {
					outputBufferSize = inputBufferSize * 4 / 3;
					outputBuffer = (char*)AOEMALLOC(outputBufferSize);
					uint32_t* ob = (uint32_t*)outputBuffer;
					for(size_t i=0, j=0; i<inputBufferSize; i+=3, j++) {
						xassert(j*4 < outputBufferSize, "Conversion buffer overflow.");
#ifdef BIG_ENDIAN
						ob[j] = (uint32_t)inputBuffer[i] << 24 | (uint32_t)inputBuffer[i+1] << 16 | (uint32_t)inputBuffer[i+2] << 8;
#else
						ob[j] = (uint32_t)inputBuffer[i] | (uint32_t)inputBuffer[i+1] << 8 | (uint32_t)inputBuffer[i+2] << 16;
#endif
					}
				} return true;
			}
		} break;
	}

	return false; // Cannot make conversion
}

bool ImageSource::expand(const char* inputBuffer, size_t inputX, size_t inputY, int inputFormat, char* &outputBuffer, size_t &outputX, size_t &outputY, size_t &outputBufferSize) {
	switch(inputFormat) {
		case IMAGE_R8G8B8A8:
		case IMAGE_A8R8G8B8: {
			outputX = nextPowerofTwo(inputX);
			outputY = nextPowerofTwo(inputY);
			outputBufferSize = outputX * outputY * sizeof(uint32_t);
			outputBuffer = (char*)AOEMALLOC(outputBufferSize);
			bzero(outputBuffer, outputX * outputY * sizeof(uint32_t));
			uint32_t* ib = (uint32_t*)inputBuffer;
			uint32_t* ob = (uint32_t*)outputBuffer;
			for(size_t y=0; y<inputY; y++) {
				for(size_t x=0; x<inputX; x++) {
					*(ob++) = *(ib++);
				}
				uint32_t lastValue = ob[-1];
				for(size_t x=inputX; x<outputX; x++) {
					*(ob++) = lastValue;
				}
			}
			for(size_t y=inputY; y<outputY; y++) {
				ib -= inputX;
				for(size_t x=0; x<inputX; x++) {
					*(ob++) = *(ib++);
				}
				uint32_t lastValue = ob[-1];
				for(size_t x=inputX; x<outputX; x++) {
					*(ob++) = lastValue;
				}
			}
		} return true;
	}
	return false;
}

// QC:A (TODO : implement getPitch for all known formats)
size_t ImageSource::getPitch(int format, int width) {
	switch(format) {
		case IMAGE_R8G8B8:
			return width * 3;
			break;
		case IMAGE_R8G8B8A8:
			return width * 4;
			break;
		case IMAGE_G8:
			return width;
			break;
		case IMAGE_G8A8:
			return width * 2;
			break;
	}
	xwarn(false, "Queried the pitch of an unknown format %d.", format);
	return 0;
}

}

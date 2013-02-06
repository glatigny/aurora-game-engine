#ifndef VFS__BUFFER_IMAGE_SOURCE_H
#define VFS__BUFFER_IMAGE_SOURCE_H

#include "image_source.h"

namespace VFS {

class BufferImageSource : public ImageSource {
private:
	char* buffer;
	int format;
	int width;
	int height;
public:
	BufferImageSource(char* buffer, int format, int width, int height);
	virtual ~BufferImageSource();

	/* ImageSource */

	virtual BufferImageSource* copy();
	virtual int getPreferredFormat();
	virtual void load(char* &buffer, Rect &bufferSize, size_t& bufferLen, size_t& pitch, int &format, Rect &imageSize, bool forcePOT = false, bool allowConversion = false);
};

}

#endif /* VFS__BUFFER_IMAGE_SOURCE_H */

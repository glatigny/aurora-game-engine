#ifndef VFS__PNG_IMAGE_SOURCE_H
#define VFS__PNG_IMAGE_SOURCE_H

#include "../image_source.h"
#include <png.h>

namespace VFS {

class PNGImageSource : public ImageSource {
private:
public:
	PNGImageSource(File* f);
	virtual ~PNGImageSource();

	static bool handles(VFS::File* f);
#ifdef PNG_SAVE_SUPPORT
	static bool handlesWriting(VFS::File* f, int format);
	static bool save(File* f, const char* buffer, int format, int width, int height);
#endif

	/* ImageSource */
	virtual PNGImageSource* copy();
	// QC:B (TODO : implement formats)
	virtual int getPreferredFormat() { return IMAGE_R8G8B8A8; }
	virtual void load(char* &buffer, Rect &bufferSize, size_t& bufferLen, size_t& pitch, int &format, Rect &imageSize, bool forcePOT, bool allowConversion);
};

}

#endif /* VFS__PNG_IMAGE_SOURCE_H */

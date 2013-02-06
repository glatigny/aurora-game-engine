#ifndef VFS__IMAGE_SOURCE_H
#define VFS__IMAGE_SOURCE_H

#include "file.h"
#include "../rect.h"

// Unknown data format
#define IMAGE_UNKNOWN_FORMAT 0xBADF

// Blank image (pixel data ignored)
#define IMAGE_BLANK 0

// Integer RGB types
#define IMAGE_R8G8B8 1
#define IMAGE_A8R8G8B8 2
#define IMAGE_R8G8B8A8 3
#define IMAGE_R16G16B16 4
#define IMAGE_A16R16G16B16 5
#define IMAGE_R16G16B16A16 6

// 16 bit per pixel (old formats)
#define IMAGE_R5G5B5A1 16
#define IMAGE_R5G6B5 17
#define IMAGE_R4G4B4A4 18

// Compressed images
#define IMAGE_BC1 32 // DXT1
#define IMAGE_BC2 33 // DXT3
#define IMAGE_BC3 34 // DXT5
#define IMAGE_BC4 35 // Alpha only
#define IMAGE_BC5 36 // RGTC

// Floating point types
#define IMAGE_F16 48
#define IMAGE_F32 49

// Palettized types
#define IMAGE_P8_RGBA8 64
#define IMAGE_P4_RGBA8 65
#define IMAGE_P2_RGBA8 66
#define IMAGE_P1_RGBA8 67 // 1 bit with palette

// Grayscale types
#define IMAGE_G8 80
#define IMAGE_G8A8 81
#define IMAGE_A8G8 82
#define IMAGE_G16 83
#define IMAGE_G16A16 84
#define IMAGE_A16G16 85
#define IMAGE_G1 86
#define IMAGE_MONO IMAGE_G1

// Special types for collisions
#define IMAGE_BITMAP_COLLISION IMAGE_MONO // A bitfield for collisions. Each pixel is 1 bit monochrome. Bit set means no collision, Bit cleared means collision.
#define IMAGE_ENVELOPE_COLLISION 129 // Two 16-bits values per line : collision start and collision end. If start > end, collision is outside the range. If start == end == 0, no collision on the line. If start == end != 0, collision on the whole line.
#define IMAGE_2ENV_COLLISION 130 // A special RLE structure for each line. In fact, it is 2 IMAGE_ENVELOPE_COLLISION structures per line.
#define IMAGE_4ENV_COLLISION 131 // A special RLE structure for each line. In fact, it is 4 IMAGE_ENVELOPE_COLLISION structures per line.
#define IMAGE_8ENV_COLLISION 132 // A special RLE structure for each line. In fact, it is 8 IMAGE_ENVELOPE_COLLISION structures per line.

// Plateform-specific formats
#define IMAGE_PSPEC 160 // It should be treated as opaque data. Never touch the content of the buffer since it may contain pointers.

/* Palettized formats :
 * The buffer is made of the palette in the specified format,
 * for all possible colors (a P4 type will have 16 entries).
 * The format of the palette is specified in the name.
 */

typedef struct EnvelopeCollisionLine {
	uint16_t start;
	uint16_t end;
} EnvelopeCollisionLine;

typedef struct Multienv2CollisionLine {
	EnvelopeCollisionLine envelopes[2];
} Multienv2CollisionLine;

typedef struct Multienv4CollisionLine {
	EnvelopeCollisionLine envelopes[4];
} Multienv4CollisionLine;

typedef struct Multienv8CollisionLine {
	EnvelopeCollisionLine envelopes[8];
} Multienv8CollisionLine;


namespace VFS {

class ImageSource {
protected:
	File* file;
	static size_t getPitch(int format, int width);
public:
	ImageSource(File* sourceFile);
	virtual ~ImageSource();

	virtual ImageSource* copy() = 0;

	/** Returns the preferred output format for this image.
	 * This function computes the optimal output format, depending on the source file.
	 * It may trigger a header read, so it generates an i/o with potential seek.
	 * For rotating discs, calling this function right before load() should not generate a seek. However, it may generate a rotation latency.
	 * @return the preferred output format of the image.
	 */
	virtual int getPreferredFormat() = 0;

	/** Load the source.
	 * This function loads the image from the source. The buffer is allocated by the function, so it must be freed by the caller.
	 * When doing power of two expansion, the image will be placed in the upper-left corner of the buffer.
	 * Outstanding parts of the buffer will be set to transparent if the format allows transparency and results will be undefined in case of a non transparent format.
	 * @param buffer outputs the buffer storing image data.
	 * @param bufferSize outputs the size of the buffer in pixels.
	 * @param format input/output parameter : takes an input format, tries to load the image to that format. If the format is unvailable, the output format will be returned right inside the parameter. Decompressors should output in a sensible format if it has the choice between many.
	 * @param imageSize will output the size of the image.
	 * @param forcePOT if enabled, the function will always generate an image with power of two dimensions, making required expansion.
	 * @param allowConversion if enabled, the function will always convert to the requested format.
	 */
	virtual void load(char* &buffer, Rect &bufferSize, size_t& bufferLen, size_t& pitch, int &format, Rect &imageSize, bool forcePOT = false, bool allowConversion = false) = 0;

	/** Convert an image.
	 * This function converts between image formats.
	 * @param inputBuffer data to convert.
	 * @param inputBufferSize the size of the buffer to convert.
	 * @param inputFormat the format of data to convert.
	 * @param outputBuffer will output a newly allocated buffer with converted data. The caller is responsible for freeing this buffer.
	 * @param outputBufferSize will output the size of the newly created buffer.
	 * @param outputFormat the wanted output format.
	 * @return true if conversion was successful, false otherwise.
	 */
	static bool convert(const char* inputBuffer, size_t inputBufferSize, int inputFormat, char* &outputBuffer, size_t &outputBufferSize, int outputFormat);

	/** Expand an image to the next power of two.
	 * The image will be filled with remaining pixel values (like in GL_CLAMP mode), to provide proper mipmapping.
	 * @param inputBuffer data to convert.
	 * @param inputX X size of the buffer, in pixels.
	 * @param inputY Y size of the buffer, in pixels.
	 * @param inputFormat the format of the buffer.
	 * @param outputBuffer OUTPUT parameter that will give back a pointer to the expanded graphics.
	 * @param outputX OUTPUT parameter that indicates the width of the output image, in pixels.
	 * @param outputY OUTPUT parameter that indicates the height of the output image, in pixels.
	 * @param outputBufferSize OUTPUT parameter that indicates the size of the output buffer, in bytes.
	 */
	static bool expand(const char* inputBuffer, size_t inputX, size_t inputY, int inputFormat, char* &outputBuffer, size_t &outputX, size_t &outputY, size_t &outputBufferSize);

	VFS::File* source() { return file; }
};

/** Open a file as an image source.
 */
ImageSource* openImage(File* sourceFile);

/** Creates an image source from a buffer.
 */
ImageSource* createImage(char* buffer, int format, int width, int height);

/** Save an image source to a file.
 * @param destFile the file to save the source to.
 * @param source the image to be saved.
 * @return true if the image was successfully saved, false otherwise.
 */
bool saveImage(File* destFile, ImageSource* source);

}

#endif /* VFS__IMAGE_SOURCE_H */


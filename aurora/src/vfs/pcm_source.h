#ifndef VFS__PCM_SOURCE_H
#define VFS__PCM_SOURCE_H


#include "file.h"

// Integer types
#define PCM_FORMAT_8U 1 // 8 bits unsigned
#define PCM_FORMAT_8S 2 // 8 bits signed
#define PCM_FORMAT_16UBE 3 // 16 bits unsigned
#define PCM_FORMAT_16SBE 4 // 16 bits signed
#define PCM_FORMAT_16ULE 5 // 16 bits unsigned, little-endian
#define PCM_FORMAT_16SLE 6 // 16 bits signed, little-endian
#define PCM_FORMAT_16U PCM_FORMAT_16ULE // 16 bits unsigned, native byte ordering (TODO : put this in ifdefs to handle automatic switch)
#define PCM_FORMAT_16S PCM_FORMAT_16SLE // 16 bits signed, native byte ordering

// Floating point types
#define PCM_FORMAT_32F 16 // 32 bits float

// Compressed types
#define PCM_FORMAT_IMA4 32 // 4 bits IMA ADPCM

#define PCM_SOURCE_CAN_REWIND 1 // Supports rewind() to the start. Streaming should support this and resume to the oldest available buffer.
#define PCM_SOURCE_CAN_SEEK 2 // Supports seeking.
#define PCM_SOURCE_CAN_LOOP 4 // Supports loop.
#define PCM_SOURCE_CAN_GROW 8 // The size is not fixed and can grow (but not shrink). Internet radios are in that case.

#define LOOP_NONE 0
#define LOOP_FORWARD 1

namespace VFS {

class PCMSource {
public:
	virtual ~PCMSource();

	virtual PCMSource* copy() = 0;

	/** Returns the output format of the sample.
	 * Depending on the current decoder, sample formats may or may not be available.
	 * When the file is initially opened, this value is set to the native format of the decoder.
	 * Constants PCM_FORMAT_* define the possible values of the format.
	 * @return the format of the decoded stream.
	 */
	virtual int getFormat() = 0;

	/** Sets the output format of the stream.
	 * @param newFormat the new format to use for decoded data.
	 * @param allowConversion allow converting the data internally if the decoder does not support this output format. This will slow down conversion.
	 * @return the format that will be ultimately used for data output, which may differ if the requested format is not available.
	 */
	virtual int setFormat(int newFormat, bool allowConversion = false) = 0;

	/** Returns the native frequency of this sample.
	 * The frequency is expressed in Hz.
	 * @return the native frequency of this sample.
	 */
	virtual int getFrequency() = 0;

	/** Returns the number of channels of this sample.
	 * Example : 1 for mono, 2 for stereo, ...
	 * @return the number of channels of this sample.
	 */
	virtual int getChannels() = 0;

	/** Sets loop type.
	 * The internal loop system allows to see a looped sample as a continuous, infinite stream.
	 * This way, the hard task of properly looping samples is handled internally, and possibly hardware-assisted.
	 * @param loopType : 0 for no loop, 1 for normal loop. Other types may appear.
	 * @return true if the new loop mode is supported and enabled, false otherwise. setLoop(0) must always return true.
	 */
	virtual bool setLoop(int loopType) = 0;

	/** Returns capabilities of this sample.
	 * This function returns a bitmask of capabilities, to inform high-level users about what they can expect of this decoder.
	 * PCM_SOURCE_CAN_* defines filters to interpret the return value.
	 * @return a bitmask of capabilities.
	 */
	virtual size_t getCapabilities() = 0;

	/** Read samples from the source.
	 * @param outputBuffer the buffer that will receive samples.
	 * @param outputBufferLen the size of the buffer.
	 * @return number of bytes filled in the buffer. Because of rounding, it will be modulo sizeof(sample format size).
	 */
	virtual size_t read(void* outputBuffer, size_t outputBufferLen) = 0;

	/** Load the full source until eof.
	 * This function loads the source to a temporary buffer, then returns a pointer to this buffer and
	 * its size.
	 * While similar to read, its parameters are output parameters, so the value of the pointer itself will change.
	 * Remember to AOEFREE() the buffer after use.
	 * @param outputBuffer [out] the pointer to a buffer containing data
	 * @param outputBufferLen [out] the size of this buffer.
	 * @param maxOutputBufferLen the maximum size of the output buffer. Specify 0 for no limit (use carefully).
	 * If the source is a stream, load will return 2 and no buffer if no limit is set, or 1 if a limit is set.
	 * @return 0 if the source was completely read until eof, 1 if the source was partially read, 2 if the source could not be read at all.
	 */
	virtual unsigned int load(char* &outputBuffer, size_t &outputBufferLen, size_t maxOutputBufferLen) = 0;

	/** Tells whether the end of stream has been reached.
	 * When a stream is finished, subsequent read() will always return 0, but it may also indicate an error or a buffer underrun.
	 * This function only returns true when the end of the stream has been reached.
	 * @return true if the stream has reached the end, false otherwise.
	 */
	virtual bool eof() = 0;

	/** Seek in the stream.
	 * To call this function, the source must be able to seek.
	 * @param position the new position in samples.
	 * @return the real position in the stream.
	 */
	virtual size_t seek(size_t position, int whence) = 0;

	/** Extract left and right part from a stereo buffer.
	 */
	size_t unInterleaveStereo(void* inputBuffer, size_t inputBufferLen, void* leftOutput, void* rightOutput) {return 0;}

	/** Returns the size of a sample.
	 * The value is used to compute buffer sizes in function of sample count.
	 * @return the size of one sample, in bytes. Stereo samples count twice.
	 */
	unsigned int getSampleSize();
};

/** Open a file as a PCM source.
 */
#ifdef PCM_SUPPORT
PCMSource* openPCM(File* sourceFile);
#else
inline PCMSource* openPCM(File* sourceFile) { xassert(false, "PCM support is not compiled in VFS."); return NULL; }
#endif

}

#endif /* VFS__PCM_SOURCE_H */


#ifndef VFS__FILE_H
#define VFS__FILE_H

#include <vector>
class ScriptVariable;
typedef std::vector<ScriptVariable> ScriptTable;

#include <wchar.h>
#include <list>
#include <string>

namespace VFS {

/** VFS access modes.
 * These modes define what can be done with files.
 * Note that these modes describe minimum capabilities. For example, POSIX files opened in APPEND mode can be seeked and read, but
 * it is not guaranteed to work for other types of files.
 */
typedef enum AccessMode {
	MODE_CLOSED, // Just keep the file closed. Used to query path or directories.
	MODE_READ, // Can read, but not write.
	MODE_WRITE, // Can read and write. The pointer is placed at the beginning of the file. Writing will overwrite.
	MODE_APPEND, // Can write only. Only append() is guaranteed to work.
	MODE_TRUNCATE,
// Truncate the file and allow for writing. In this mode, the file is truncated, then it is opened in append mode.
} AccessMode;

class File {
	friend File* openFile(const char* filename, const File* pathRoot, AccessMode mode);
private:

protected:
	size_t mmap_size;
	AccessMode mode;
	char* pathName; // The name of the file, used to compute the relative path.

	/** Open a file relative to this one.
	 * The filename will be on the same physical source, that is, never contain "//".
	 * @param filename the file that will be opened.
	 */
	virtual File* openRelative(const char* filename, AccessMode mode) const = 0;

public:
	File() :
		mmap_size(0),
		pathName(NULL)
	{}
	File(const char* newName) :
		pathName(AOESTRDUP(newName))
	{}
	virtual ~File() {
		if(pathName)
			AOEFREE(pathName);
	}

	/** Duplicate this File object.
	 * The copy will be in MODE_CLOSED by default.
	 * @return a copy of this File object.
	 */
	virtual File* copy() = 0;

	/** Changes access mode on the fly.
	 * On most backends, this will probably close and re-open the file, which may introduce delays and/or disk seeks.
	 * Please avoid mode switchs if not necessary. Switching from MODE_CLOSED to any other mode should be safe anyway.
	 * @param newMode the new mode of the file.
	 * @return true if the mode change was successful, false otherwise.
	 */
	virtual bool switchMode(AccessMode newMode) = 0;

	/** Maps this file to a buffer.
	 * This is the preferred way to generate a buffer from a file. On selected plateforms, the file will be read on demand, rather than copied in RAM.
	 * If the plateform does not support mmap, a standard buffer will be allocated.
	 *
	 * The function uses at most as much memory space as the size parameter.
	 *
	 * In all cases, use VFS::File::munmap to free the buffer.
	 *
	 * Once a file is mmapped, calling other methods than munmap will have undefined results.
	 *
	 * @param size the size of the returned buffer.
	 * @return the buffer, or NULL if something went wrong.
	 */
	virtual void* mmap(size_t size) {
		char* buffer = (char*) AOEMALLOC(size);
		seek(0, SEEK_SET);

#ifndef NDEBUG
		size_t realLen = read(buffer, size);
		assert(realLen == size);
#endif

		mmap_size = size;

		return buffer;
	}

	/** Map the whole file into memory.
	 * @param buffer OUTPUT : address of the buffer. NULL if the operation failed.
	 * @return the size of the buffer in bytes.
	 */
	virtual size_t mmapWholeFile(char*& buffer) {
		size_t bufsize = (size_t)size();
		xerror(bufsize, "Trying to mmap an empty file or the size() operation is not supported for %s.", name());
		buffer = (char*)mmap(bufsize);
		return bufsize;
	}

	/** Frees a mmaped buffer.
	 * @param buffer the buffer as returned by mmap.
	 */
	virtual void munmap(void* buffer) {
		xerror(mmap_size, "Trying to munmap a file that has not been mmapped before.");
		munmap(buffer, mmap_size);
	}

	/** Frees a mmaped buffer.
	 * @param buffer the buffer as returned by mmap.
	 * @param size the size of the buffer as passed to mmap.
	 */
	virtual void munmap(void* buffer, size_t size) {
		AOEFREE(buffer);
	}

	/** Returns the content of the file in a newly allocated buffer.
	 * The function allocates memory via AOEMALLOC, reads the whole file into the buffer,
	 * then returns its pointer, or NULL if something went wrong.
	 * The returned buffer must be AOEFREE() by the caller.
	 * If this function is successful, it is guaranteed that File::size() equals the size of the returned buffer.
	 * @return the content of the file in a newly allocated buffer.
	 */
	virtual char* readToBuffer();

	/** Returns the content of the file in a newly allocated zero-terminated string.
	 * The function allocates memory via AOEMALLOC, reads the whole file into the buffer,
	 * then returns its pointer, or NULL if something went wrong.
	 * The returned buffer must be AOEFREE() by the caller.
	 * If this function is successful, it is guaranteed that the size of the returned buffer is equal to File::size() + 1 and is zero-terminated.
	 * @return the file content in a zero-terminated buffer of size()+1 bytes.
	 */
	virtual char* readToString();

	/** Parse the file in records.
	 * For text files, the file is read line by line.
 	 * For directories, each entry contains a file name.
 	 * @return a list of variables containing each entry of the file.
 	 */
	virtual ::ScriptTable* parse();

	// Read and seek

	/** Tells whether this file can be read.
	 * @return true if the file can be read, false otherwise.
	 */
	virtual bool canRead() {
		return false;
	}

	/** Read a part of the file.
	 * @param buffer destination buffer.
	 * @param bufferLen the size of the buffer.
	 * @return number of bytes read.
	 */
	virtual size_t read(void* buffer, size_t bufferLen) = 0;

	/** Seek in a file.
	 * @param position the position to seek to.
	 * @return true if the seek was successful, false otherwise.
	 */
	virtual bool seek(off_t position, int whence = SEEK_SET) {
		return false;
	}

	/** Tells whether this file has random access.
	 * @return true if the file can be seeked, false otherwise.
	 */
	virtual bool canSeek() {
		return false;
	}

	/** Tells whether this file can seek to a particular position.
	 * Cases when this function returns false whereas canSeek() returns true :
	 *  - The position is beyond end of file
	 *  - The file can only seek to position 0 and the parameter was not 0.
	 *  - The file is a stream and position is outside the buffer.
	 *
	 * @param position the position to seek to.
	 * @return true if a seek to that position would succeed, false otherwise.
	 */
	virtual bool canSeek(off_t position) {
		return false;
	}

	/** Tells whether this file can seek to a particular position.
	 * Cases when this function returns false whereas canSeek() returns true :
	 *  - The position is beyond end of file
	 *  - The file can only seek to position 0 and the parameter was not 0.
	 *  - The file is a stream and position is outside the buffer.
	 *
	 * @param position the position to seek to.
	 * @param whence SEEK_SET, SEEK_CUR or SEEK_END.
	 * @return true if a seek to that position would succeed, false otherwise.
	 */
	virtual bool canSeek(off_t position, int whence) {
		if(whence == SEEK_CUR) {
			position += tell();
		} else if(whence == SEEK_END) {
			off_t s = size();
			if(position >= s)
				return false;
			position = size() - position;
		}
		return canSeek(position);
	}

	/** Tells whether this file can be written to.
	 * Some file types only support append.
	 * @see append()
	 * @return true if the file can be written to, false otherwise.
	 */
	virtual bool canWrite() {
		return false;
	}

	/** Tells whether you can append data to this file.
	 * Some file types only support overwrite.
	 * @see write()
	 * @return true if you can append data, false otherwise.
	 */
	virtual bool canAppend() {
		return false;
	}

	/** Writes to the current position of the file pointer.
	 * This function erases already existing data at the same position.
	 * Note that files may have a size limit and some media like memory card blocks cannot grow nor shrink.
	 * @param buffer data to write to the file.
	 * @param bufferLen size of the buffer to write.
	 * @return the number of bytes written to the file.
	 */
	virtual size_t write(const void* buffer, size_t bufferLen) = 0;

	/** Appends data to a file.
	 * This function appends data to a file. Please use this function instead of write if you do not need to overwrite.
	 * Some file types may support append but not write.
	 */
	virtual size_t append(const void* buffer, size_t bufferLen) = 0;

	/** Truncate a file to a given position.
	 * This function truncates a file in order to shrink it.
	 * Please note that this function may fail on some device. You must not assume that bytes will be zero after position.
	 * Although this operation may fail, it is recommended to call it before closing the file to ensure that its size will be minimal.
	 * @param position the position to truncate the file to.
	 * @return true if the file was successfully truncated to the correct position, false otherwise.
	 */
	virtual bool truncate(off_t position = 0) = 0;

	/** Tells whether the file is in an end of file state.
	 * The function only returns true on true end of file cases. In particular, there are cases when this function can return false and read() would return 0.
	 * For example, this function will return false when a buffer underrun is encountered for a streamed file.
	 * @return true if the end of file has been reached, false otherwise.
	 */
	virtual bool eof() = 0;

	/** Get the current position in the file.
	 * @return the current position of the read pointer in the file.
	 */
	virtual off_t tell() = 0;

	/** Get the size of the file.
	 * @return the size of the file, in bytes, or 0 if the size is unknown.
	 */
	virtual off_t size() {
		return 0;
	}

	/** Get the path and the name of the file.
	 * @return the path and the name of the file, relative to its container, or NULL if the name was not set.
	 */
	virtual char* name() const {
		return pathName;
	}

	/** Get the relative path of the file.
	 * @return the path of the file, relative to its container.
	 */
	virtual const char* path() { assert(false); return NULL; }

	/** Get the name of the file.
	 * @return the name of the file.
	 */
	virtual const char* basename() { assert(false); return NULL; }

	/** Get the extension of the file.
	 * @return the extension of the file, with its leading dot or an empty string if no extension.
	 */
	virtual const char* extension() { assert(false); return NULL; }

};

class Directory: public File {
public:
	Directory(const char* filename) :
		File(filename)
	{
	}
	// Disable file semantics
	virtual void* mmap() {
		assert(false);
		return false;
	}
	virtual void munmap() {
		assert(false);
	}
	virtual size_t read(void* buffer, size_t bufferLen) {
		assert(false);
		return 0;
	}
	virtual bool seek(off_t position, int whence) {
		assert(false);
		return false;
	}
	virtual bool canSeek() {
		assert(false);
		return false;
	}
	virtual bool canSeek(off_t position, int whence) {
		assert(false);
		return false;
	}
	virtual bool canAppend() {
		assert(false);
		return false;
	}
	virtual bool truncate(off_t position) {
		assert(false);
		return false;
	}
	virtual bool eof() {
		assert(false);
		return true;
	}
	virtual off_t tell() {
		assert(false);
		return 0;
	}
	virtual off_t size() {
		assert(false);
		return 0;
	}
	virtual size_t write(const void* buffer, size_t bufferLen) {
		assert(false);
		return 0;
	}
	virtual size_t append(const void* buffer, size_t bufferLen) {
		assert(false);
		return 0;
	}
	virtual ::ScriptTable* parse() = 0;

	virtual std::list<std::string> listFiles(const char* extension = NULL, bool recursive = false) = 0;

	virtual Directory* copy() = 0;
};

/** Open a file relative to a reference.
 * The function opens a file relative to the reference. If you have no reference, use VFS::root.
 * @param reference the file to be used as a reference.
 * @param filename the name of the file to open, relative to this one.
 * @return the file descriptor of the opened file.
 */
extern File* openFile(const char* filename, const File* pathRoot = NULL, AccessMode mode = MODE_CLOSED);

}

#endif /* VFS__FILE_H */

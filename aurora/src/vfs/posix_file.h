#ifndef VFS__POSIX_FILE_H
#define VFS__POSIX_FILE_H

#include "file.h"

#include <stdio.h>


namespace VFS {

class PosixFile : public File {
private:
	FILE* f;
	off_t fileSize; // Only for MODE_READ files.

	bool setMode(AccessMode newMode);

protected:

	/* File */
	File* openRelative(const char* filename, AccessMode mode) const;

public:
	PosixFile(const char* filename, AccessMode mode);
	virtual ~PosixFile();

	virtual PosixFile* copy();

	/* File */

	size_t read(void* buffer, size_t bufferLen);
	bool seek(off_t position, int whence);
	bool canSeek() { return mode != MODE_APPEND && mode != MODE_CLOSED; }
	bool canSeek(off_t position) { return canSeek() && (position <= size()); }
	bool canRead() { return mode == MODE_READ || mode == MODE_WRITE || mode == MODE_TRUNCATE; }
	bool canWrite() { return mode == MODE_WRITE || mode == MODE_TRUNCATE; }
	bool canAppend() { return mode == MODE_APPEND; }
	bool eof() { assert(f); return feof(f); }
	off_t size();
	off_t tell() { xassert(mode != MODE_CLOSED && f, "No file descriptor"); return ftell(f); }
	size_t write(const void* buffer, size_t bufferLen) { assert(f && canWrite()); return fwrite(buffer, 1, bufferLen, f); }
	size_t append(const void* buffer, size_t bufferLen) { assert(f && canAppend()); if(canSeek()) seek(0, SEEK_END); return fwrite(buffer, 1, bufferLen, f); }
	bool truncate(off_t size) { assert(false); return false; } // TODO implement truncate
	bool switchMode(AccessMode newMode);

};

class PosixDirectory : public Directory {
friend File* openFile(const File* reference, const char* filename);
protected:
	File* openRelative(const char* filename, AccessMode mode) const;
public:
	PosixDirectory() : Directory("") {}
	PosixDirectory(const char* filename) : Directory(filename) { assert(isDirectory(filename)); }

	virtual PosixDirectory* copy();

	/* Directory */

	virtual std::list<std::string> listFiles(const char* extension, bool recursive);

	/* File */

	virtual ScriptTable* parse();
	bool switchMode(AccessMode newMode) { return false; }
};

#ifdef POSIX_SUPPORT
extern const PosixDirectory* root();
#endif

}

#endif /* VFS__POSIX_FILE_H */


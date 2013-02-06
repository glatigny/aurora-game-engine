#include "posix_file.h"
#include "../script_variable.h"

#define VFS_POSIX_READ_BLOCK 4096

namespace VFS {

PosixFile::PosixFile(const char* filename, AccessMode newMode) :
	File(filename), f(NULL), fileSize(0)
{
	setMode(newMode);
}

PosixFile::~PosixFile() {
	xassert((f != NULL || mode == MODE_CLOSED) && ! (f != NULL && mode == MODE_CLOSED), "Inconsistent file state.");
	if(mode != MODE_CLOSED)
		fclose(f);
}

PosixFile* PosixFile::copy() {
	return new PosixFile(name(), MODE_CLOSED);
}

bool PosixFile::setMode(AccessMode newMode) {
	switch(newMode){
	case MODE_READ:
		f = fopen(name(), "rb");
		break;
	case MODE_WRITE:
		f = fopen(name(), "r+b");
		break;
	case MODE_APPEND:
		f = fopen(name(), "ab");
		break;
	case MODE_TRUNCATE:
		f = fopen(name(), "w+b");
		break;
	case MODE_CLOSED:
		mode = MODE_CLOSED;
		return true;
	}

	if(!f) {
		syslog("PosixFile VFS module : cannot open %s", name());
		return false;
	}

	mode = newMode;

	if(mode == MODE_READ) {
		fseek(f, 0, SEEK_END);
		fileSize = ftell(f);
		rewind(f);
	}

	return true;
}

bool PosixFile::switchMode(AccessMode newMode) {
	if(newMode == mode)
		return true;

	xassert((f != NULL || mode == MODE_CLOSED) && ! (f != NULL && mode == MODE_CLOSED), "Inconsistent file state.");

	// MODE_CLOSED is trivial

	if(mode == MODE_CLOSED) {
		return setMode(newMode);
	}

	if(newMode == MODE_CLOSED) {
		fclose(f);
		f = NULL;
		mode = MODE_CLOSED;
		return true;
	}

	// Bypass re-opening for compatible modes
	
	if(newMode == MODE_READ && (mode == MODE_WRITE || mode == MODE_TRUNCATE)) {
		seek(0, SEEK_SET);
		mode = newMode;
		return true;
	}

	if(newMode == MODE_APPEND && (mode == MODE_WRITE || mode == MODE_TRUNCATE)) {
		seek(0, SEEK_END);
		mode = newMode;
		return true;
	}

	if(newMode == MODE_TRUNCATE && mode == MODE_WRITE) {
		seek(0, SEEK_SET);
		truncate(0);
		mode = newMode;
		return true;
	}

	// Re-open file for incompatible modes

	fclose(f);
	return setMode(newMode);
}

off_t PosixFile::size() {
	if(mode == MODE_READ) {
		return fileSize;
	}

	if(mode == MODE_CLOSED)
		f = fopen(name(), "rb");
	xerror(f, "Could not open file %s.", name());
	off_t current = ftello(f);
	fseeko(f, 0, SEEK_END);
	fileSize = ftello(f);
	fseeko(f, current, SEEK_SET);
	if(mode == MODE_CLOSED) {
		fclose(f);
		f = NULL;
	}

	return fileSize;
}

File* PosixFile::openRelative(const char* filename, AccessMode mode) const {
	const char* finalName = relPath(name(), filename);
	if(isDirectory(finalName)) {
		return new PosixDirectory(finalName);
	}

	return new PosixFile(finalName, mode);
}

// QC:S
size_t PosixFile::read(void* outputBuffer, size_t bufferLen) {
	char* buffer = (char*) outputBuffer;

	assert(canRead());

	/*XXX*/
	/*
	 size_t blocksRead = 0;
	 if(bufferLen > VFS_POSIX_READ_BLOCK) {
	 blocksRead = fread(buffer, VFS_POSIX_READ_BLOCK, bufferLen/VFS_POSIX_READ_BLOCK, f);
	 buffer += blocksRead*VFS_POSIX_READ_BLOCK;
	 bufferLen -= blocksRead*VFS_POSIX_READ_BLOCK;
	 }
	 size_t lastBlock = fread(buffer, bufferLen, 1, f);*
	 return blocksRead*VFS_POSIX_READ_BLOCK + lastBlock*bufferLen;
	 //*/
	return fread(buffer, 1, bufferLen, f);
}

bool PosixFile::seek(off_t position, int whence) {
	assert(canSeek(position));
	return fseeko(f, position, whence) == 0 ? true : false;
}

PosixDirectory* PosixDirectory::copy() {
	return new PosixDirectory(name());
}

File* PosixDirectory::openRelative(const char* filename, AccessMode mode) const {
	static char finalName[4096];

	if(!*name()) {
		strncpy(finalName, filename, 4096);
	} else {
		// Compute finalName by appending / and filename.
		int filenameLen = strlen(name());
		if(filenameLen > 4096)
			filenameLen = 4096;
		memcpy(finalName, name(), filenameLen);
		finalName[filenameLen] = '/';
		strncpy(finalName + filenameLen + 1, filename, 4096 - (filenameLen + 1));
	}
	finalName[4095] = '\0';

	if(!*finalName)
		return new PosixDirectory(".");

	if(isDirectory(finalName)) {
		return new PosixDirectory(finalName);
	}

	return new PosixFile(finalName, mode);
}

ScriptTable* PosixDirectory::parse() {
	xassert(name(), "Directory has no path information.");
	DIR* dir = opendir(name());
	struct dirent* dp;
	xassert(dir, "Could not open directory %s.", name());

	ScriptTable* result = new ScriptTable;

	while((dp = readdir(dir))) {
		ScriptVariable v;
		v.setval(dp->d_name);
		result->push_back(v);
	}

	closedir(dir);
	return result;
}

// QC:A (TODO:implement recursive lookup)
std::list<std::string> PosixDirectory::listFiles(const char* extension, bool recursive) {
	xassert(name(), "Directory has no path information.");
	DIR* dir = opendir(name());
	struct dirent* dp;
	xassert(dir, "Could not open directory %s.", name());

	std::list<std::string> result;

	while((dp = readdir(dir))) {
		const char* file = dp->d_name;
		const char* extdot = strrchr(file, '.');
		if(!extension || (extdot && strcmp(extdot, extension) == 0)) {
			result.push_back(file);
		}
	}

	closedir(dir);
	return result;
}

#if VFS_ROOT_DIRECTORY == PosixDirectory
const PosixDirectory* root() {
	static PosixDirectory* r = NULL;
	if(!r)
		r = AOENEW PosixDirectory();
	return r;
}
#endif

}

#include "file.h"
#include "posix_file.h"
#include "../script_variable.h"

namespace VFS {

File* openFile(const char* filename, const File* reference, AccessMode mode) {
	if(!filename) {
		return NULL;
	}

	const char* sepPosition = strstr(filename, "//");

	if(sepPosition) {
#ifdef POSIX_SUPPORT
		if(sepPosition == filename) {
			return openFile(sepPosition+2, root(), mode);
		} else
#endif

#ifdef PACKAGE_SUPPORT
		if(sepPosition-filename > 4 && !strncmp(sepPosition-4, ".aop", 4)) {
			char packageName[sepPosition-filename];
			memcpy(packageName, filename, sepPosition-filename);
			return openFile(sepPosition+2, new PackageFile(reference, packageName) /* FIXME : Leak */, mode);
		} else
#endif

		{
			syslog("Invalid file access specification : %s", filename);
			assert(false);
		}
	} else {
#ifdef POSIX_SUPPORT
		if(!reference) {
			reference = root();
		}
#endif
		assert(reference);
		File* relative = reference->openRelative(filename, mode);
		xerror(relative, "No file %s relative to %s.", filename, reference->name());
		return relative;
	}

	xwarn(false, "Could not open %s.", filename);
	return NULL;
}

::ScriptTable* File::parse() {
	xassert(canRead(), "Cannot read file.");
	xassert(size(), "File is empty or its size is not known.");

	::ScriptTable* t = new ScriptTable;

	char* b = readToString();

	char* line = b;
	for(;;) {
		char* lineEnd = strchr(line, '\n');
		if(!lineEnd)
			break;
		if(lineEnd > line + 1 && lineEnd[-1] == '\r') {
			lineEnd[-1] = '\0';
		} else {
			*lineEnd = '\0';
		}

		ScriptVariable v;
		v.setval(line);
		t->push_back(v);

		line = lineEnd + 1;
	}

	AOEFREE(b);

	return t;
}

char* File::readToBuffer() {
	AccessMode oldMode = mode;
	switchMode(MODE_READ);
	if(canSeek(0, SEEK_SET)) {
		seek(0, SEEK_SET);
	}

	char* b = (char*)AOEMALLOC(size());
	if(!b)
		switchMode(oldMode);
		return NULL;
	if(!read(b, size())) {
		xwarn(false, "Could not read file %s to a buffer", name());
		AOEFREE(b);
		switchMode(oldMode);
		return NULL;
	}
	if(oldMode == MODE_CLOSED)
		switchMode(oldMode);
	return b;
}

char* File::readToString() {
	AccessMode oldMode = mode;
	switchMode(MODE_READ);
	if(canSeek(0, SEEK_SET)) {
		seek(0, SEEK_SET);
	}

	char* b = (char*)AOEMALLOC(size() + 1);
	if(!b) {
		switchMode(oldMode);
		return NULL;
	}
	if(!read(b, size())) {
		xwarn(false, "Could not read file %s to a string", name());
		AOEFREE(b);
		switchMode(oldMode);
		return NULL;
	}

	b[size()] = '\0';

	xwarn((off_t)strnlen(b, size()) == size(), "Warning : the file %s has NUL characters, it may not be a text file. File is %d bytes and string is %d bytes.", name(), (int)size(), strnlen(b, size()));

	if(oldMode == MODE_CLOSED)
		switchMode(oldMode);
	return b;
}

}


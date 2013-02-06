#include "vfs_file.h"
#include "context.h"
#include "state.h"

#define HEADER vfs_file
#define HELPER scriptable_object_cpp
#include "include_helper.h"

VfsFile::VfsFile(State* attachedState, VFS::File* file) : Abstract(attachedState->engine()), b(NULL), f(file?file->copy():NULL) {}
VfsFile::VfsFile(AuroraEngine* parentEngine, VFS::File* file) : Abstract(parentEngine), b(NULL), f(file?file->copy():NULL) {}
VfsFile::VfsFile(AuroraEngine* parentEngine, const char* fullPath, VFS::File* fileRoot) : Abstract(parentEngine), b(NULL), f(VFS::openFile(fullPath, fileRoot)) {}

VfsFile::~VfsFile() {
	if(f)
		delete f;
	if(b)
		AOEFREE(b);
}

ScriptTable* VfsFile::parse() {
	switchMode(VFS::MODE_CLOSED);
	return f->parse();
}

void VfsFile::switchMode(VFS::AccessMode newMode) {
	f->switchMode(newMode);
}


bool VfsFile::isDirectory() {
	switchMode(VFS::MODE_CLOSED);

	xassert(f, "No file descriptor.");

	return dynamic_cast<VFS::Directory*>(f) != NULL;
}


char* VfsFile::readFile() {
	switchMode(VFS::MODE_READ);

	size_t s = (size_t)f->size();
	if(b)
		AOEFREE(b);
	b = (char*)AOEMALLOC(s + 1);

	if(f->canSeek(0, SEEK_SET)) {
		f->seek(0, SEEK_SET);
	}

	f->read(b, s);
	b[s + 1] = '\0';

	return b;
}


void VfsFile::writeFile(const char* data) {
	switchMode(VFS::MODE_TRUNCATE);
	f->write(data, strlen(data));
}


void VfsFile::append(const char* data) {
	switchMode(VFS::MODE_APPEND);
	f->append(data, strlen(data));
}

// QC:?
VfsFile* VfsFile::open(const char* filename) {
	return new VfsFile(engine(), filename, f);
}

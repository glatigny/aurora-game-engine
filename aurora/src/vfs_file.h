#ifndef VFS_FILE_H
#define VFS_FILE_H

#include "vfs.h"
#include "scriptable_object.h"
#include "script_variable.h"

/** Provides boxing for VFS::File in the script engine.
 */
class VfsFile : public Abstract, public ScriptableObject
{
protected:
	char* b;
	VFS::File* f;
	VFS::AccessMode mode;

	void switchMode(VFS::AccessMode newMode);

public:
	VfsFile() : b(NULL), f(NULL), mode(VFS::MODE_CLOSED) {}
	VfsFile(State* attachedState, VFS::File*);
	VfsFile(AuroraEngine* parentEngine, VFS::File*);
	VfsFile(AuroraEngine* parentEngine, const char* fullPath, VFS::File* fileRoot);
	virtual ~VfsFile();

#define HEADER vfs_file
#define HELPER scriptable_object_h
#include "include_helper.h"

	/** Tell if this file is a directory.
	 * @return true if the file is a directory, false otherwise.
	 */
	bool isDirectory();

	/** Read the whole file in a table.
	 * For files, each entry is a line.
	 * For directories, each entry is a file name.
	 * @return the content of the file in a table.
	 */
	ScriptTable* parse();

	/** Read a text file as a whole.
	 * The string will be null-terminated.
	 * @return the first null-terminated string from the file.
	 */
	char* readFile();

	/** Write a string to the file.
	 * The content of the file is replaced by the content of the parameter.
	 * @param data the string to write to the file.
	 */
	void writeFile(const char* data);

	/** Append a string to a file.
	 * @param data the string to append.
	 */
	void append(const char* data);

	/** Open a file relative to another.
	 * @param filename the name of the file to open.
	 * @return the VfsFile specified by filename, relative to this one.
	 */
	VfsFile* open(const char* filename);

	/** Return the name of the file.
	 * @ return the name of the file.
	 */
	const char* name() {
		return f?f->name():"";
	}

	/* AOEObject */

	/** Debox the enclosed object.
	 * @return the boxed VFS::File variable.
	 */
	virtual VFS::File* source() { return f; }
};

#endif

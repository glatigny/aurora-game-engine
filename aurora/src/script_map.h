#ifndef SCRIPT_MAP_H
#define SCRIPT_MAP_H

#include <map>
#include "vfs.h"

#define LOAD_BLOCK_SIZE 4096


class ScriptMap {
private:
	struct strCmp {
		bool operator()(VFS::File* a, VFS::File* b) const {
			return strcmp(a->name(), b->name()) > 0;
		}
	};

	class Script {
	private:

	public:
		VFS::File* scriptFile;
		char* code;
		unsigned long length;
		int useCounter;

		Script() :
			scriptFile(NULL),
			code(NULL),
			length(0),
			useCounter(0)
		{}

		Script(const Script& s);
		Script(VFS::File* scriptFile);

		bool operator==(const ScriptMap::Script& other) const {
			return strcmp(other.scriptFile->name(), scriptFile->name()) == 0;
		}

		bool operator>=(const ScriptMap::Script& other) const {
			return strcmp(other.scriptFile->name(), scriptFile->name()) >= 0;
		}

		bool operator<=(const ScriptMap::Script& other) const {
			return !(other > *this);
		}

		bool operator>(const ScriptMap::Script& other) const {
			return strcmp(other.scriptFile->name(), scriptFile->name()) > 0;
		}

		bool operator<(const ScriptMap::Script& other) const {
			return !(other >= *this);
		}
	};

	std::map<VFS::File*, ScriptMap::Script, strCmp> scripts;

public:
#ifndef NDEBUG
	~ScriptMap();
#endif
	/** Get the pointer to a script.
	 */
	char* getScriptCode(VFS::File* scriptFile);

	/** Free a script.
	 * @return true if the script is still in use, false otherwise.
	 */
	bool freeScript(VFS::File* scriptFile);

	/** Free a script based on its buffer.
	 * @param code A pointer to the buffer holding the code as returned by getScriptCode.
	 * @return true if the script is still in use, false otherwise.
	 */
	bool freeScriptByBuffer(const char* code);

	/** Free a script based on a pointer inside it.
	 * @param code A pointer to the buffer holding the code as returned by getScriptCode.
	 * @return true if the script is still in use, false otherwise.
	 */
	bool freeScriptByPointer(const char* ptr) {
		return freeScriptByBuffer(getBasePointer(ptr));
	}

	/** Get the starting point of a script.
	 * @param ptr A pointer pointing anywhere in any buffer.
	 * @return the pointer to the starting point of the buffer containing the pointer, or NULL if the pointer is outside all loaded scripts.
	 */
	char* getBasePointer(const char* ptr);
};

#endif /* SCRIPT_MAP_H */


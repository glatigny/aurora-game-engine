#include "script_map.h"
#include <assert.h>
#include <stdio.h>

#ifndef NDEBUG
ScriptMap::~ScriptMap() {
	for(std::map<VFS::File*, ScriptMap::Script, strCmp>::iterator si = scripts.begin(); si != scripts.end(); si++) {
		syslog(" Script : %s", si->first->name());
	}
	xwarn(scripts.size() == 0, "Script map not empty : script allocation has memory leaks.");
}
#endif

ScriptMap::Script::Script(const Script& s) : length(s.length), useCounter(s.useCounter) {
	if(s.length) {
		scriptFile = s.scriptFile;
		code = s.code;
	}
}

char* ScriptMap::getScriptCode(VFS::File* scriptFile) {
	std::map<VFS::File*, ScriptMap::Script, strCmp>::iterator si = scripts.find(scriptFile);

	if(si != scripts.end()) {
		si->second.useCounter++;
		return si->second.code;
	}

	syslog("Loading script %s", scriptFile->name());

	ScriptMap::Script script(scriptFile);
	scripts[scriptFile->copy()] = script;
	return script.code;
}

bool ScriptMap::freeScript(VFS::File* scriptFile) {
	std::map<VFS::File*, ScriptMap::Script, strCmp>::iterator si = scripts.find(scriptFile);
	assert(si != scripts.end());

	ScriptMap::Script& s = si->second;
	s.useCounter--;
	if(s.useCounter == 0) {
		delete s.scriptFile;
		AOEFREE(s.code);
		VFS::File* s_index = si->first;
		scripts.erase(si);
		delete s_index;

		syslog("Unloading script %s", scriptFile->name());

		return false;
	}

	return true;
}

char* ScriptMap::getBasePointer(const char* ptr) {
	for(std::map<VFS::File*, ScriptMap::Script, strCmp>::iterator si = scripts.begin(); si != scripts.end(); si++) {
		char*& code = si->second.code;
		if(ptr >= code && ptr <= code+si->second.length) { // <= because the pointer may point the ending \0
			return code;
		}
	}

	return NULL;
}

ScriptMap::Script::Script(VFS::File* newScriptFile) : useCounter(1) {
	scriptFile = newScriptFile->copy();
	length = scriptFile->size();
	code = scriptFile->readToString();
}



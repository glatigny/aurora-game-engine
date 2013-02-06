#include "sub_scriptable.h"
#include "aurora_engine.h"
#include "script_map.h"
#define HEADER sub_scriptable
#define HELPER scriptable_object_cpp
#include "include_helper.h"

SubScriptable::SubScriptable() : scriptContext(NULL), scriptSource(NULL) {}

SubScriptable::SubScriptable(Context* superContext, VFS::File* scriptFileName) {
	scriptContext = new Context(superContext->engine());
	scriptContext->setsuper(superContext);

	if(scriptFileName) {
		scriptSource = scriptFileName->copy();
	} else {
		scriptSource = NULL;
	}
}

// QC:?
SubScriptable::~SubScriptable() {
	freeScript();
	if(scriptContext)
		delete scriptContext;
}

void SubScriptable::initScript() {
	assert(scriptContext);

	scriptContext->registerScriptVariable("this")->setval(this);

	if(scriptSource) {
		char* code = engine()->scriptMap().getScriptCode(scriptSource);
		scriptContext->load_script(scriptSource, code);
		scriptContext->framestep();
	}
}

void SubScriptable::freeScript() {
	xwarn(isFinished(), "Trying to forcefully unload a running script : %s.", scriptSource?scriptSource->name():"Unknown script"); // Protect from dead pointer. Not fully secure, but better than nothing.

	if(scriptSource) {
		scriptContext->engine()->scriptMap().freeScript(scriptSource);
		delete scriptSource;
	}
}

void SubScriptable::loadScript(VFS::File* filename) {
	freeScript();
	scriptSource = filename->copy();
	initScript();
}

Context* SubScriptable::script() {
	return scriptContext;
}

void SubScriptable::framestep() {
	if(scriptContext)
		scriptContext->framestep();
}

bool SubScriptable::isFinished() {
	return !scriptContext || !scriptContext->isRunning();
}


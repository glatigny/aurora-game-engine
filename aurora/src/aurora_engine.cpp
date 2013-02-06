#include <iterator>
#include "aurora_engine.h"
#include "script_monitor.h"
#include "controllable.h"
#include "aoe_object.h"

// QC:P
AuroraEngine::AuroraEngine() : dataRootDirectory(NULL) {
	initContext();
	assert(checkByteOrder());
}

// QC:P
AuroraEngine::AuroraEngine(HardwareEngine* hwEng, VFS::File* datadir) {
	hardwareEngine = hwEng;
	dataRootDirectory = datadir->copy();
	assert(checkByteOrder());
	initContext();
}

// QC:G (manque le idleScheduler)
AuroraEngine::~AuroraEngine() {
//	assert(idleScheduler != NULL);

	syslog("Deleting still active states");
	for(std::list<State*>::iterator si = states.begin(); si != states.end(); si++) {
		syslog("|-> Still active state : %p", (AOEObject*)*si);
		delete *si;
	}

	syslog("Deleting the HardwareEngine");
	if(hardwareEngine) delete hardwareEngine;

	syslog("Deleting script monitor");
	delete scriptContext->getVar("@mon");

	syslog("Deleting main scriptContext");
	delete scriptContext;

	syslog("Deleting root directory object");
	if(dataRootDirectory)
		delete dataRootDirectory;
		
	syslog("Aurora Engine shut down");
}

// QC:P
void AuroraEngine::initContext() {
	scriptContext = new Context(this);
	scriptContext->registerAllNativeClasses();
	ScriptMonitor* sm = new ScriptMonitor(this);
	scriptContext->setval("@mon", sm);
	scriptContext->registerScriptVariable("displaySize")->setval(hardware()->getDisplaySize()->getW(), hardware()->getDisplaySize()->getH());
}

// QC:?
void AuroraEngine::setVersionInformation(const char* project) {
	assert(scriptContext);
	Context* info = new Context(this);
	info->setval("PROJECT", (char*) project);
	info->setval("VENDOR", (char*) AURORA_ENGINE_VENDOR);
	info->setval("VERSION", (char*) AURORA_ENGINE_VERSION);
	info->setval("REVISION", (char*) AURORA_ENGINE_REVISION);
#ifndef NDEBUG
	info->setval("DEBUG", true);
#else
	info->setval("DEBUG", false);
#endif
	info->setval("UPDATE_URL", (char*) ( AURORA_PROJECT_URL "/update.aop//main.aos" ));
	info->setval("ENGINE_URL", (char*) AURORA_ENGINE_URL);
	info->setval("PROJECT_URL", (char*) AURORA_ENGINE_URL);
	ScriptVariable* infoVar = scriptContext->registerScriptVariable("ENGINE");
	infoVar->setval(info);
}

State* AuroraEngine::pushNewState(VFS::File* stateScript) {
	// Generate a new state.
	State* st;
	st = new State(this, true, true, true);
	push(st);

	// Load the script.
	if(stateScript) {
		st->loadScript(stateScript);
	}
	return st;
}

// QC:P
void AuroraEngine::push(State* state) {
	assert(state != NULL);
	states.push_front(state);
}

// QC:P
void AuroraEngine::pop() {
	states.pop_front();
}

// QC:P
void AuroraEngine::remove(State* state) {
	assert(state != NULL);
	states.remove(state);
}

// QC:?
int AuroraEngine::run(int framesToRun) {
	dur elapsedTime;
	int elapsedFrames = 0;
	bool activeStates = true;

#ifdef PERFLOG
	int lastRenderTime = 0;
	int lastUpdateTime = 0;
	int lastTotalTime = 1;

	int renderTime = 0;
	int updateTime = 0;
	int totalTime = 0;

	int freq = 2;

	State perfState(this, true, true, true);
#endif

	while(!hardware()->terminationRequest() && activeStates && ( (framesToRun < 0) || (elapsedFrames++ < framesToRun) )) {
		hardware()->preDraw();
		elapsedTime = hardware()->getElapsedTime();

#ifdef PERFLOG
		int start = getTime();
#endif

		// Do graphical rendering
		renderStates();

#ifdef PERFLOG
		int render = getTime();
#endif

		// Get input status
		hardware()->processInput();

		// Update states
		activeStates = runStates(elapsedTime);

		// Flush the limbo
		flushLimbo();

		// Upload background tasks to the worker thread
		hardware()->doBackgroundTasks();

#ifdef PERFLOG
		int update = getTime();
		renderTime += render - start;
		updateTime += update - render;
		totalTime += elapsedTime;
		if(!--freq) {
			freq = 3;
			lastUpdateTime = updateTime;
			updateTime = 0;

			lastRenderTime = renderTime;
			renderTime = 0;

			lastTotalTime = totalTime;
			totalTime = 0;
		}

		hardware()->displayStats();
		hardware()->displayMeter(1, 0, lastRenderTime*1000/lastTotalTime, 0, 1000);
		hardware()->displayMeter(1, 1, lastUpdateTime*1000/lastTotalTime, 0, 1000, 0);
#endif

		hardware()->postDraw();
		hardware()->syncToFrameRate();
	}

	return elapsedFrames;
}

// QC:?
bool AuroraEngine::runStates(dur elapsedTime) {
	std::list<State*>finishedStates;

	scriptContext->setval("currentFPS", (int)(hardware()->getCurrentFPS())); // TODO : Implement float/double in the script engine.
	scriptContext->setval("elapsedTime", (int)(elapsedTime));
#ifdef MEMTRACKING
	scriptContext->setval("aoeMallocTotal", aoeMallocTotal);
	scriptContext->setval("aoeMallocPeak", aoeMallocPeak);
	scriptContext->setval("aoeMallocCount", aoeMallocCount);
	scriptContext->setval("aoeVidAllocTotal", aoeVidAllocTotal);
	scriptContext->setval("aoeVidAllocPeak", aoeVidAllocPeak);
	scriptContext->setval("aoeVidAllocCount", aoeVidAllocCount);
#endif

	bool exclusive = false;
	for(std::list<State*>::iterator si = states.begin(); si != states.end(); si++) {
		exclusive = !runState(*si, elapsedTime);
		if((*si)->isFinished()) {
			syslog("State %p finished.", (AOEObject*)(*si));
			finishedStates.push_front(*si);
		}
		if(exclusive) {
			break;
		}
	}

	int tmpFps = scriptContext->getint("targetFPS"); // TODO : Implement float/double in the script engine.
	if( tmpFps != 0 )
	{
		hardware()->setFrameRate(tmpFps);
		scriptContext->resetval("targetFPS");
	}

	// Delete finished states
	for(std::list<State*>::iterator si = finishedStates.begin(); si != finishedStates.end(); si++) {
		states.remove(*si);
		delete *si;
	}
	finishedStates.clear();

	return !states.empty();
}

// QC:?
void AuroraEngine::renderStates() {
	// Draw states from deepest to nearest for proper stacking.
	for(std::list<State*>::iterator si = states.end(); si != states.begin(); ) {
		--si;
		renderState(*si);
	}
}

// QC:G
bool AuroraEngine::runState(State* s, int elapsedTime) {
	assert(s != NULL);
	assert(elapsedTime >= 0);

	if(s->enabled) {
		s->update(elapsedTime);
	}

	return !s->exclusive;
}

// QC:P
void AuroraEngine::renderState(State* s) {
	if(hardwareEngine != NULL) {
		s->render();
	}
}

// QC:?
void AuroraEngine::flushLimbo() {
	if(!limbo.empty()) {
		std::list<AOEObject*> pending(limbo);
		limbo.clear();
		for(std::list<AOEObject*>::iterator it = pending.begin(); it != pending.end(); it++) {
			delete *it;
		}
	}
}

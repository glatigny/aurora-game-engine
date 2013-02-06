#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

class AuroraEngine;
class AOEObject;

#include <list>
#include <set>

#include "interface.h"
#include "state.h"
#include "hardware_engine.h"
#include "graphical_effects.h"
#include "script_map.h"
#include "context.h"
#include "vfs.h"

/** Manages the different states of the game.
 * This class is the entry point to the Aurora Object Engine.
 *
 * The engine can work in fully automated mode and provide a main loop.
 * It can also be used as a controlled process and individual tasks can
 * be separated (sub mode).
 *
 * Main mode usage :
 * Just create a HardwareEngine, push a state and call run().
 * run() is the main loop provided by the engine. It will stabilize
 * to the targetFPS value and return when the last state has finished.
 * You can run the engine for a limited time by providing a parameter
 * to the run function. You can call run() again to resume the engine.
 * Beware that framerate stabilizer will be disturbed if you resume after
 * much time has passed. If you need long pauses, use the sub mode.
 *
 * Sub mode usage :
 * Sub mode gives you full control over the main loop and the framerate
 * stabilizer. Sub mode can be used to make many engines run in parallel,
 * even on the same HardwareEngine !!!
 * Please note that some methods are linked to the hardware engine.
 * Thus, they must be called once for each HardwareEngine, not once per
 * AuroraEngine. If you have only one hardwareEngine for multiple AuroraEngine
 * instances, you will have to make sure that these methods are called once per
 * frame.
 */
class AuroraEngine : public Interface {
private:
	std::list<AOEObject*> limbo;
	std::list<State*> states;
	HardwareEngine* hardwareEngine;
	GraphicalEffects defaultEffect;
	//EventScheduler* idleScheduler;
	Context* scriptContext;
	VFS::File* dataRootDirectory;
	ScriptMap scriptMapObject;

	bool runState(State* s, int elapsedTime);
	void renderState(State* s);
	void initContext();

public:
	AuroraEngine();
	AuroraEngine(HardwareEngine* hwEng, VFS::File* datadir);
	~AuroraEngine();

	/** Sets version information variables.
	 * @param project the name of the project.
	 */
	void setVersionInformation(const char* project);

	/** Add a state into the state stack and load a script.
	 * @param script the script to load into the new state.
	 * @return the newly created state.
	 */
	State* pushNewState(VFS::File* stateScript = NULL);

	/** Add a state into the state stack.
	 * @param state the state to be added.
	 */
	void push(State* state);

	/** Pop the last state from the stack.
	 * The state will be deleted and its destructor called.
	 */
	void pop();

	/** Remove the given state of the stack.
	 * The state will be deleted and its destructor called.
	 * @param state the state to remove from the stack.
	 */
	void remove(State* state);

	/** Get the hardware engine linked to this AuroraEngine.
	 * @return a pointer to the current hardware engine. May return NULL if no hardware is present.
	 */
	HardwareEngine* hardware() {
		return hardwareEngine;
	}

	/** Get the current idle calls scheduler.
	 * @return a pointer to the current idle EventScheduler.
	 */
	/*EventScheduler* getIdleScheduler() {
		return idleScheduler;
	}*/

	/** Run the states until the state stack becomes empty.
	 * This is the integrated main loop of the engine.
	 * @param framesToRun limit running time to framesToRun frames.
	 */
	int run(int framesToRun = -1);

	/** Make the states run for elapsedTime milliseconds.
	 * This function updates all states of the engine.
	 * @param elapsedTime number of milliseconds elapsed between this update and the one before.
	 * @return true if states are still active, false otherwise.
	 */
	bool runStates(dur elapsedTime);

	/** Renders states to the current hardware engine.
	 * You must call HardwareEngine::preDraw() and HardwareEngine::postDraw()
	 * respectively before and after calling this function.
	 */
	void renderStates();

	/** Return the current data root directory.
	 */
	VFS::File* datadir() { return dataRootDirectory; }

	/** Return the current script context.
	 */
	Context* script() { return scriptContext; }

	/** Return the script map.
	 * Used to load scripts.
	 */
	ScriptMap& scriptMap() { return scriptMapObject; }

	/** Set an object as pending for deletion.
	 * Objects will wait for their fate in the limbo. The limbo is cleared after each frame.
	 * param object the object to be deleted at next frame.
	 */
	void doom(AOEObject* object) { limbo.push_back(object); }

	/** Flush the limbo.
	 */
	void flushLimbo();

};

#endif


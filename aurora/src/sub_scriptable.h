#ifndef SUB_SCRIPTABLE_H
#define SUB_SCRIPTABLE_H

class ScriptMap;

#include "scriptable_object.h"
#include "context.h"
#include "blocking.h"

/** A SubScriptable object is an object which embeds a script.
 * It has a script context which allows either to store objects in a flexible
 * way or to execute scripts on external actions.
 *
 * Script context is accessible via external scripts, to call functions or
 * to access to internal variables.
 *
 * @see Context.
 */
class SubScriptable: virtual public ScriptableObject, public Blocking {
private:
	Context* scriptContext;
protected:
	VFS::File* scriptSource;
public:
#define HEADER sub_scriptable
#define HELPER scriptable_object_h
#include "include_helper.h"
	SubScriptable();
	SubScriptable(Context* supercontext, VFS::File* scriptSource);
	virtual ~SubScriptable();

	/** Initializes the script context.
	 * This function sets the "this" variable up and runs the initialization script.
	 */
	virtual void initScript();

	/** Returns the internal script context.
	 * The internal script context is the context storing functions and
	 * variables specific to this object.
	 * Please note that it will always return the top context of the
	 * object, and that you do not have access to running or interrupted
	 * subcontexts.
	 * @return the internal topcontext of this object.
	 */
	Context* script();

	/** Makes the subcontext run for one frame.
	 */
	virtual void framestep();

	/** Tells whether the object has finished.
	 *
	 * A SubScriptable is considered terminated if it has no threads
	 * and no blocking calls pending.
	 *
	 * @return true if the SubScriptable has finished, false otherwise.
	 */
	virtual bool isFinished();

	/** Loads a new script and run it inside this SubScriptable.
	 * WARNING:Using this function if the script has already run can leave
	 * variables pointing to the freed script.
	 * @param filename the file that will be loaded.
	 */
	void loadScript(VFS::File* scriptFile);

	/** Free the script from the script map.
	 */
	void freeScript();
};
#endif


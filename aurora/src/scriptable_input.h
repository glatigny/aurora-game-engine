#ifndef SCRIPTABLE_INPUT_H
#define SCRIPTABLE_INPUT_H

#include "state.h"
#include "signalable.h"
#include "controllable.h"
#include "controller_state.h"
#include "sub_scriptable.h"

/** A Scriptable Input is the object that make the link between inputs and the script
 */
class ScriptableInput : public Signalable, public Controllable, public SubScriptable {
private:

protected:

public:
#define HEADER scriptable_input
#define HELPER scriptable_object_h
#include "include_helper.h"
	ScriptableInput() {}
	ScriptableInput(State* attachedState, VFS::File* sourceFile);
	virtual ~ScriptableInput() {}

	VFS::File* source() { return ScriptableObject::source(); }
	
	/** 
	 *
	 */
	virtual int onSignal(ScriptableObject* other, SignalId signalId, int parameter);

	/* Signalable */
	
	virtual int signal(ScriptableObject* other, SignalId signal, int data);

	/* Controllable */

	virtual void processInput(ControllerState* state);
};

#endif

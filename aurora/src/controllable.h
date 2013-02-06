#ifndef CONTROLLABLE_H
#define CONTROLLABLE_H

#include "entity.h"
#include "controller_state.h"

/** A controllable object which can react to user input.
 */
class Controllable : virtual public Entity {
public:
	Controllable() {}
	Controllable(State* newAttachedState) : Entity(newAttachedState) {}

	virtual ~Controllable() {}

	/** Process user input.
	 * The object will update itself accordying to the controller state.
	 * @param state the state of the user input.
	 */
	virtual void processInput(ControllerState* state) = 0;

	bool isInstance(const char* className) {
		return ( strcmp(className, "Controllable") == 0);
	}
};

#endif


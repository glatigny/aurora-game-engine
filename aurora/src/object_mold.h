#ifndef OBJECT_MOLD_H
#define OBJECT_MOLD_H

class Context;
class ScriptVariable;
class State;

#ifndef SCRIPTABLE_OBJECT_H
#error You must include scriptable_object.h instead of object_mold.h
#endif // SCRIPTABLE_OBJECT_H

#include <list>
#include <map>
#include "abstract.h"

class ObjectMold : public Abstract, public ScriptableObject {
protected:
	char* objectId;
public:
#define HEADER object_mold
#define HELPER scriptable_object_h
#include "include_helper.h"
	ObjectMold() : objectId(NULL) {}
	ObjectMold(AuroraEngine* parentEngine) : Abstract(parentEngine), objectId(NULL) {}
	virtual ~ObjectMold() { if(objectId) AOEFREE(objectId); }

	/** Mold an object.
	 * This function will generate an object with the parameters defined when
	 * the mold has been created. The attached state is passed as a parameter,
	 * so a single mold can instantiate objects for multiple different states.
	 * @param state the state to attach the object to.
	 * @return the instance of the created object.
	 */
	virtual ScriptableObject* create(State* state);

	/** Set the id of this object.
	 * Normally used internally, this can be handy in a few scripts.
	 * It sets the id as if it were changed in the AOD.
	 * @param newObjectId the new id of the object.
	 */
	void setObjectId(const char* newObjectId) {
		assert(newObjectId);
		if(objectId)
			AOEFREE(objectId);
		objectId = AOESTRDUP(newObjectId);
	}

	/** Internal use.
	 * Used by helpers/scriptable_object_*
	 */
	virtual ScriptableObject* createInstance(State* state, ownedset* owned) { assert(false); return NULL; } // Should be pure virtual.

	/** Duplicate this mold.
	 * @return a deep copy of this mold.
	 */
	virtual ObjectMold* copy() { assert(false); return NULL; }
};

#endif /* OBJECT_MOLD_H */


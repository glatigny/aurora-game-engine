#include "scriptable_object.h"
#include "state.h"

ScriptableObject* ObjectMold::create(State* state) {
	ownedset* owned = new ownedset;
	ScriptableObject* instance = createInstance(state, owned);
	instance->setOwnedObjectSet(owned);
	return instance;
}

#define HEADER object_mold
#define HELPER scriptable_object_cpp
#include "include_helper.h"


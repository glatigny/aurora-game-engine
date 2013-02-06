#include "cameraman.h"

#define HEADER cameraman
#define HELPER scriptable_object_cpp
#include "include_helper.h"

Cameraman::Cameraman(State* newAttachedState) {
	assert(newAttachedState);

	attachedState = newAttachedState;
}


#include "entity.h"
#include "state.h"
#include "vfs.h"

void Entity::setAttachedState(State* newAttachedState) {
	attachedState = newAttachedState;
}

AuroraEngine* Entity::engine() {
	return attachedState->engine();
}

VFS::File* Entity::source() {
	return state()->source();
}


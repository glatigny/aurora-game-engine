#include "realtime.h"
#include "state.h"
	
Realtime::Realtime(State* attachedState) : Entity(attachedState), updateEnabled(true) {
	if((AOEObject*)attachedState != (AOEObject*)this)
		attachedState->registerRealtimeObject(this);
}

Realtime::~Realtime() {
	if((AOEObject*)attachedState != (AOEObject*)this && state()) {
		state()->unregisterRealtimeObject(this);
	}
}


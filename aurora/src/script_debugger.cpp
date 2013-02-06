#include "script_debugger.h"
#include "state.h"
#include "aurora_engine.h"
#include "hardware_engine.h"
#include "controller_state.h"

#ifndef MOUSE_SUPPORT
#error SCRIPT_DEBUGGER_SUPPORT needs MOUSE_SUPPORT
#endif

bool ScriptDebugger::scriptDebugStep(char* &ptr, Context* ec) {
	if(ptr != NULL) {
		char l_near[60];
		memcpy(l_near, ptr - 20, sizeof(l_near) - 1);
		for(unsigned int i = 0; i < sizeof(l_near); i++)
			if((i < 20 || l_near[i] > 0) && l_near[i] < 32)
				l_near[i] = ' ';
		l_near[sizeof(l_near) - 1] = '\0';
		syslog("|      \"%s\"", l_near);
		syslog("|___________________________T");
	}

	ControllerState* cs = NULL;
	State* s = dynamic_cast<State*>(ec->getobj("state"));
	if(!s) {
		syslog("Script debugger : cannot step : cannot reach current state.");
		return false;
	}

	HardwareEngine* hw = s->engine()->hardware();

	while(!cs || !cs->isMouseButtonPressed(0)) {
		cs = hw->getControllerState();
		if(cs->isMouseButtonPressed(1))
			return true;
	}

	return false;
}


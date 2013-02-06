#include "input_engine.h"

InputEngine::InputEngine() {
	
}

InputEngine::~InputEngine() {
	// Delete controllers.
	for(std::vector<ControllerState*>::iterator i = controllers.begin(); i !=  controllers.end(); i++) {
		if(*i) {
			delete *i;
		}
	}
}


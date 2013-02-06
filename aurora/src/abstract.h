#ifndef ABSTRACT_H
#define ABSTRACT_H

#include "aoe_object.h"

class AuroraEngine;

/** Abstract is the main interface for all intangible objects, which are not linked to a State.
 */
class Abstract : virtual public AOEObject {
protected:
	AuroraEngine* parentEngine;

	Abstract() : parentEngine(NULL) {}

	Abstract(AuroraEngine* newParentEngine) : parentEngine(newParentEngine) {
		xassert(parentEngine, "Object %p is not attached to any engine.", (AOEObject*)this);
	}

public:

	/* AOEObject */
	
	AuroraEngine* engine() {
		return parentEngine;
	}

	virtual ~Abstract() { }
};
#endif


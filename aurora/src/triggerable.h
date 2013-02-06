#ifndef TRIGGERABLE_H
#define TRIGGERABLE_H

#include "entity.h"

/** An object that can process events and process tasks synchronously.
 */
class Triggerable : public Entity {
public:
	virtual ~Triggerable();
	
	/** Process an event that has been triggered
	 * @param event the event to process.
	 */
	virtual void trigger(int value) = 0;
};

#endif


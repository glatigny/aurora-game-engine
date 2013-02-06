#ifndef RESTARTABLE_H
#define RESTARTABLE_H

#include "interface.h"

/** A simple interface to provide the restart() method.
 */
class Restartable : virtual public Interface {
public:
	virtual ~Restartable() {}

	/** Restart the object.
	 * This function has various possible effects, but it should put the object in a state ressembling to its instanciation state. Only the internal state must be reset : object references and effects must be left intact.
	 */
	virtual void restart() {}
};

#endif /* RESTARTABLE_H */

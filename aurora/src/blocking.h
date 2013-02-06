#ifndef BLOCKING_H
#define BLOCKING_H

#include "interface.h"

/* An interface to provide isFinished() and other rendezvous methods.
 * It also provides many synonyms for ease of use and code clarity.
 */
class Blocking : public Interface {
private:

public:
	virtual ~Blocking() {}

	/** Tells whether this object has finished.
	 * @return true if the object has finished, false otherwise.
	 */
	virtual bool isFinished() = 0;

	/** Tells whether this object has finished.
	 * @return true if the object has finished, false otherwise.
	 */
	bool hasFinished() {
		return isFinished();
	}

	/** Tells whether this object must be waited for.
	 * @return true if the object is requesting blocking, false otherwise.
	 */
	bool wait() {
		return !isFinished();
	}

	bool isBlocking() {
		return wait();
	}

	bool isBlocked() {
		return isBlocking();
	}
};

#endif /* BLOCKING_H */

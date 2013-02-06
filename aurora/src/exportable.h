#ifndef EXPORTABLE_H
#define EXPORTABLE_H

#include "scriptable_object.h"
#include <tinyxml.h>

/** This interface exposes methods to export the state of objects.
 * This exportation is different from serialization because sub-objects are not
 * altered. Only the most important variables are exported.
 * The main use of this interface is to replicate the object through the
 * network.
 */
class Exportable : virtual public ScriptableObject {
private:

public:

	virtual ~Exportable() {
	}

	/** Export the status of the object.
	 * This function must be very quick. It will be used in CPU-bound applications and realtime parts
	 * of the engine. Please avoid too many virtual calls (especially getters/setters). You are allowed
	 * to cheat with friend, direct member access and other low-level dirty tricks.
	 * @return the state of the object as a packed structure.
	 * @see getClassFormat().
	 */
	virtual char* exportStatus() const = 0;

	/** Import a previously exported status.
	 * @param savedStatus the status of this object as exported by exportStatus.
	 * @see exportStatus().
	 */
	virtual void importStatus(const char* savedState) = 0;

	/** Returns the format of the packed structure used for this class.
	 * This function will be overloaded and will return a string used to manipulate
	 * the buffer.
	 */
	virtual const char* getClassFormat() const = 0;
};

#endif


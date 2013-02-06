#ifndef AOE_OBJECT_H
#define AOE_OBJECT_H

#include <set>
#include <vector>

class AuroraEngine;
class AOEObject;
typedef std::vector<AOEObject*> ownedset;
namespace VFS {
	class File;
}

#include <assert.h>
#include "interface.h"

class AOEObject : virtual public Interface {
private:
	AOEObject* owner; // Object that loaded this one
	ownedset* owned; // Objects loaded by this one
public:
	AOEObject() : owner(NULL), owned(NULL) {}
	AOEObject(AOEObject* newOwner) : owned(NULL) { newOwner->own(this); }
	virtual ~AOEObject();

	void setOwnedObjectSet(ownedset* owned);

	// QC:A
	/** Internal use only.
	 * Used by State::load and ObjectMold::create
	 */
	static void addOwnedObject(ownedset* owned, AOEObject* object) {
		xassert(owned, "NULL owned set.");
		xassert(object, "Trying to own NULL object.");
		//TODO : xassert(owned->find(object) == owned->end(), "Trying to own an object which is already owned.");
		owned->push_back(object);
	}
	
	/** Tells whether the object owns another.
	@param object the object to test.
	@return true if object is owned by this object, false otherwise.
	*/
	bool owns(AOEObject* object);

	/** Return the engine linked to this object.
	 */
	virtual AuroraEngine* engine() = 0;

	/** Register owned object.
	@param owned : the object to register for automatic destruction.
	*/
	void own(AOEObject* object);

	/** Unregister owned object.
	@param owned : the object to unregister.
	*/
	void unregisterOwned(AOEObject* owned);

	/** Delete the object if it is owned.
	This function is NOT a magical swiss army knife to free memory.
	It must be matched to its corresponding own() call. Like delete is paired to new, free is paired to AOEMALLOC, mayDelete is paired to own().
	It will only delete objects owned by the current object ("this"). If the object is not owned, it is left untouched.
	NULL pointers will be ignored.
	@param object the object to try to delete.
	*/
	void mayDelete(AOEObject* object);

	/** Mark this object as pending for deletion.
	 * The object will be deleted at the next frame loop.
	 */
	void doom();

	virtual VFS::File* source() = 0;
};


#endif


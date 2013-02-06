#include "aurora_engine.h"
#include "aoe_object.h"

AOEObject::~AOEObject() {
	if(owned)
		for(ownedset::iterator i = owned->begin(); i != owned->end(); i++) {
			xassert(*i, "Owned NULL object.");
			(*i)->owner = NULL; // Protect from double-delete
			// FIXME delete *i;
		}
		delete owned;

	if(owner)
		owner->unregisterOwned(this);
}

// QC:B does not own children.
void AOEObject::own(AOEObject* object)
{
	xerror(object != this, "Object cannot own itself");
	xwarn(!object->owner, "Object already has an owner");

	if(object->owner) {
		if(object->owner == this)
			return;
		// Steal object from previous owner.
		object->owner->unregisterOwned(object);
	}

	if(!owned)
	{
		owned = new ownedset;
	}

	owned->push_back(object);
	object->owner = this;
}

// QC:?
bool AOEObject::owns(AOEObject* object) {
	for(ownedset::iterator i = owned->begin(); i != owned->end(); i++) {
		if(*i == object)
			return true;
	}
	return false;
}

// QC:?
void AOEObject::unregisterOwned(AOEObject* ownedObject)
{
	xerror(ownedObject != this, "Object cannot own itself");
	xerror(ownedObject->owner == this, "Object owner information is wrong");
	xerror(owned, "Object has no owned sub-objects");
	ownedObject->owner = NULL;
	ownedset& s = *owned;

	int removed = 0;

	for(ownedset::iterator i = s.begin(); i != s.end(); i++) {
		xassert(*i, "Owned NULL object.");
		while(*i == ownedObject) {
			int size = s.size();
			if(size > 1 && *i != s[size-1])
				*i = s[size-1];
			else
				*i = NULL;
			removed++;
		}
	}

	for(int i=0; i<removed; i++)
		s.pop_back();
}

// QC:?
void AOEObject::setOwnedObjectSet(ownedset* newOwned) {
	xassert(!owned, "Cannot reset owned set");
	owned = newOwned;
	for(ownedset::iterator i = owned->begin(); i != owned->end(); i++) {
		xassert(*i, "Owned NULL object.");
		(*i)->owner = this;
	}
}

// QC:F (won't delete sub-objects in most cases)
void AOEObject::mayDelete(AOEObject* object) {
	if(!object)
		return;
	xassert(owned, "An object is marked as owned, but was not registered to its owner.");
	ownedset& s = *owned;
	int size = s.size();
	for(int i=0; i<size; i++) {
		if(s[i] == object) {
			AOEObject* toBeDeleted = s[i];
			xassert(toBeDeleted->owner == this, "Wrong object owner.");
			if(size > 1)
				s[i] = s[size-1];
			s.pop_back();
			// FIXME delete toBeDeleted;
			return;
		}
	}
}

// QC:P
void AOEObject::doom()  {
	xassert(engine(), "Cannot doom this object : it is not linked to the engine and lost its way to the limbo.");
	engine()->doom(this);
}
#ifndef INTERFACE_H
#define INTERFACE_H

#include "aurora_mem.h"

/** An interface defines common operator overloading and common reflexion API to be used for introspective parts of the engine.
 * This class has no specific dependencies outside from the system and global headers.
 */

class Interface {

#ifdef INTROSPECTION_SUPPORT
protected:
	Interface* self; // Pointer to itself, to have the real base address.

public:
	Interface() : self(this) {}
#endif /* INTROSPECTION_SUPPORT */

#ifndef NDEBUG
public:
	void* operator new(size_t s) {
#ifndef ALLOW_REGULAR_NEW
		xadvice(false, "Calling \"new\" to create Aurora objects is invalid. Please use AOENEW instead.");
#endif
		return AOEMALLOC(s);
	}

	void operator delete(void* p) {
		AOEFREE(p);
	}
#else /* NDEBUG */
#ifndef ALLOW_REGULAR_NEW
private:
	void* operator new(size_t s) {
		return NULL;
	}
#else
public:
	void* operator new(size_t s) {
		return AOEMALLOC(s);
	}

	void operator delete(void* p) {
		AOEFREE(p);
	}
#endif
#endif /* NDEBUG */

public:
	void* operator new(size_t s, enum Aurora::Mem::Hint hint, const char* file, int line) {
		return AOEMALLOC(s);
	}

};

#endif /* INTERFACE_H */

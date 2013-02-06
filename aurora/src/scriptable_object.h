#ifndef SCRIPTABLE_OBJECT_H
#define SCRIPTABLE_OBJECT_H

#include <vector>
#include <iostream>

#ifdef KEEP_AOD_SOURCES
#include "tinyxml.h"
#endif

class Context;
class ScriptVariable;
namespace VFS {
	class File;
}

#include "aoe_object.h"
#include "vfs.h"

/** This class must be implemented by all scriptable objects.
It provides necessary methods to make function and class registering working.
*/
class ScriptableObject : virtual public AOEObject {
friend class Factory;
private:
#ifdef KEEP_AOD_SOURCES
	TiXmlElement* aodSource;
#endif
public:
#ifdef KEEP_AOD_SOURCES
	ScriptableObject() : aodSource(NULL), sourceFile(NULL) {}
#else
	ScriptableObject() : sourceFile(NULL) {}
#endif
	ScriptableObject(VFS::File* newSourceFile);

	VFS::File* sourceFile;

	virtual ~ScriptableObject();
	
	/** Returns the name of the class.
	@return constant pointer to the name of the class.
	@see is_instance
	*/
	virtual const char* getClassName();

	/** Returns the size of this object, in bytes.
	 * @return the real memory size of the object.
	 */
	virtual const size_t sizeOf() { return sizeof(ScriptableObject); }

	/** Tells if an object is an instance of a given class.
	This method takes inheritance into account.
	@param className : name of the class to be tested.
	@return true if the object is an instance of class_name.
	*/
	virtual bool isInstance(const char* className);

	/** Registers all functions in a context.
	This function adds necessary variables to be able to call the functions of this objects
	from a script.
	This function is called automatically by context.
	@param v the Context into which functions are registered.
	*/
	virtual void registerFunctions(Context& v);

#ifdef KEEP_AOD_SOURCES
	/** Returns the AOD element that has been used to load the object.
	@return the AOD element that has been used to load the object.
	*/
	TiXmlNode* aod() { return aodSource; }

	/** Saves the data back to its AOD file.
	@param file the AOD file to save data to.
	*/
	virtual void saveAOD(VFS::File* file);
	virtual void saveAOD() { saveAOD(source()); }
#endif

	/** Return the data file of this object.
	 */
	virtual VFS::File* source() { return sourceFile; }
};

/** Safe cast for scriptable objects.*/
template<class C>
inline C scriptable_object_cast(ScriptableObject* obj) {
	if(!obj) return NULL;

	C classObj = dynamic_cast<C>(obj);
	if(!classObj) {
		xwarn(classObj, "%s cannot be cast to the requested type, returning NULL.", obj->getClassName());
		return NULL;
	}

	return classObj;
}

#include "context.h"
#include "object_mold.h"
#ifdef KEEP_AOD_SOURCES
#include "vfs_file.h"
#endif

#endif /* SCRIPTABLE_OBJECT_H */

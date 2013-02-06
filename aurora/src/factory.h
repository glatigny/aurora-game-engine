#ifndef FACTORY_H
#define FACTORY_H

/** Factory allow dynamic object creation from xml data
 */

class Factory;
class ScriptableObject;
class State;
class InterpolationData;
class ObjectMold;
class GraphicalEffects;
class InterpolationDescriptor;

#include <tinyxml.h>
#include <map>
#include <vector>
#include <set>
#include <list>
#include <iostream>

#include "aoe_object.h"
namespace VFS {
	class File;
}


#ifdef FACTORY_EXTENSION
#include FACTORY_EXTENSION
#endif

class Factory
{
public:
	struct strCmp
	{
		bool operator()(const char* s1, const char* s2) const
		{ return strcmp(s1, s2) < 0; }
	};
	typedef ScriptableObject* (Factory::*objectfactory)(TiXmlElement*, VFS::File*, ownedset*);
	typedef ObjectMold* (Factory::*moldfactory)(TiXmlElement*, VFS::File*);
	typedef std::map<const char*, objectfactory, strCmp> mapping;
	typedef std::map<const char*, moldfactory, strCmp> moldmapping;
	typedef std::map<char*, char*> charmap;
	typedef std::map<char*, int> intmap;
	typedef std::map<char*, ObjectMold*> moldmap;
	typedef std::map<char*, ScriptableObject*> objectmap;
private:
	static mapping factories;
	static moldmapping moldFactories;
	State* state;
	AuroraEngine* aurora_engine;
public:
	Factory(State* attachedState);
	~Factory();
#define LIST_HELPER factory_h
#include "xh_list.xh"

	/** Unregister all factories available.
	 */
	static void unregisterFactories();

	/** Register all factories available.
	 */
	static void registerFactories();

	/** Register a class creation method
	 * @param classid class identifier
	 * @param factory method to create instance of class
	 */
	static void registerFactory(const char* classid, objectfactory factory, moldfactory moldFactory);
	
	/** Instanciate a class from XML data.
	 * It returns an instance of an object with the same class as its XML tag.
	 * @param xmlData xml flow to init instance
	 */ 
	ScriptableObject* createInstance(TiXmlElement *xmlData, VFS::File* xmlFile, ownedset* owned);

	ObjectMold* createMold(TiXmlElement *xmlData, VFS::File* xmlFile);

	/** Load the first element of a file with attribute overriding.
	 * Used by State::load
	 * @param filename the file to load.
	 * @param overrides a map of all overrides to apply to the attributes.
	 * @param owned a set of owned objects.
	 * @return an instance of the first element found in the file.
	 */
	ScriptableObject* load(VFS::File* xmlFile, std::map<const char*, const char*>* overrides, ownedset* owned);

	/** Load a GraphicalEffects from a node.
	 * WARNING : it creates a new instance of the structure.
	 * Don't forget to delete it after use.
	 * @param xmlData the XML node holding effect attributes.
	 * @return the GraphicalEffects corresponding to attributes found in the node. It returns NULL if the "effects" attribute is false.
	 */
	static GraphicalEffects loadEffects(TiXmlElement *xmlData);

	/** Load interpolations from a node.
	 * WARNING : it creates a new instance of the class.
	 * Don't forget to delete it after use.
	 * @param xmlData the XML node holding effect attributes.
	 * @return the InterpolatioDescriptor corresponding to attributes found in the node.
	 */
	InterpolationDescriptor* loadInterpolations(TiXmlElement* xmlData);
	
	/** Load an object from a node.
	 * @param The parent node containing the object.
	 * @paraam propertyName the name of the tag containing the object.
	 * @return an instance of the object.
	 */
	ScriptableObject* loadObject(TiXmlElement *xmlData, const char* propertyName, VFS::File* pathRoot, ownedset* owned);
	
	/** Load all the nodes of a file.
	 * @param filename the file to load.
	 * @param destination the vector that will receive pointers to loaded objects.
	 */
	void loadAllNodes(VFS::File* xmlFile, std::vector<ScriptableObject*>* destination, ownedset* owned);

	/** Load all named nodes of a file and put them in a map.
	 * It uses the "name" attribute to get the string part of the map.
	 * @param filename the file to load.
	 * @param destination the map that will receive pointers to loaded objects, associated to their names.
	 */
	void loadNodeMap(VFS::File* xmlFile, std::map<const char*, ScriptableObject*>* destination, ownedset* owned);
	
	ObjectMold* loadMold(VFS::File* xmlFile, std::map<const char*, const char*>* overrides = NULL);

	/** Load all the nodes of a file.
	 * @param filename the file to load.
	 * @param destination the vector that will receive pointers to loaded objects.
	 */
	void loadAllNodeMolds(VFS::File* xmlFile, std::list<ObjectMold*>* destination);

	/** Load all named nodes of a file and put them in a map.
	 * It uses the "name" attribute to get the string part of the map.
	 * @param filename the file to load.
	 * @param destination the map that will receive pointers to loaded objects, associated to their names.
	 */
	void loadNodeMapMolds(VFS::File* xmlFile, moldmap* destination);
};

#endif

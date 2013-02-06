#include "factory.h"
#include "renderable_interpolator.h"
#include "macros/pointer_utils.h"
#include "object_mold.h"
#include "h_list.h"
#include <memory>

// Include all loadable classes' xh here
#define LIST_HELPER factory_cpp
#include "xh_list.xh"

// Include mold functions here
#define LIST_HELPER factory_mold_cpp
#include "xh_list.xh"

Factory::mapping Factory::factories;
Factory::moldmapping Factory::moldFactories;

Factory::Factory(State* attachedState) {
	state = attachedState;
	aurora_engine = state->engine();
	registerFactories();
}

Factory::~Factory() {
	unregisterFactories();
}

void Factory::unregisterFactories()
{
	for(mapping::iterator i = factories.begin(); i != factories.end(); i++) {
		AOEFREE((char*)i->first);
	}
	for(moldmapping::iterator i = moldFactories.begin(); i != moldFactories.end(); i++) {
		AOEFREE((char*)i->first);
	}
	
	factories.clear();
	moldFactories.clear();
}

void Factory::registerFactories()
{
	if(factories.empty()) {
// Loadable classes' xh must be included a second time
#define LIST_HELPER factory_registration_cpp
#include "xh_list.xh"
	}
}

void Factory::registerFactory(const char* classid, objectfactory factory, moldfactory moldFactory)
{
	factories[(const char*)AOESTRDUP(classid)] = factory;
	moldFactories[(const char*)AOESTRDUP(classid)] = moldFactory;
}

// QC:A
ScriptableObject* Factory::createInstance(TiXmlElement *xmlData, VFS::File* xmlFile, ownedset* owned)
{
	Factory::mapping::iterator it;

	if(!xmlData) {
		return NULL;
	}

	const char* id = xmlData->Attribute("id");
	if(id) {
		ScriptableObject* obj = state->script()->getobj(id);
		if(obj) {
			syslog("reusing id=%s", id);
			return obj;
		}
	}

	const char* src = xmlData->Attribute("src");
	if(src) {
		std::map<const char*, const char*> overrides;
		TiXmlAttribute* attr = xmlData->FirstAttribute();
		while(attr) {
			if(strcmp(attr->Name(), "id")!=0 && strcmp(attr->Name(), "src")!=0) {
				overrides[attr->Name()] = attr->Value();
			}
			attr = attr->Next();
		}

		VFS::File* srcFile = VFS::openFile(src, xmlFile);
		ScriptableObject* createdObject = load(srcFile, &overrides, owned);
		delete srcFile;
		
		if(id) {
			state->script()->setval(id, createdObject);
		}

		return createdObject;
	}

	// Find the factory for this class and call it.
	it = factories.find(xmlData->Value());
	xassert(it != factories.end(), "Don't know how to make instances of %s", xmlData->Value());
	xassert(it->second, "NULL factory for class %s", xmlData->Value());

	// Call the class-specific factory
	ScriptableObject* createdObject = (this->*(it->second))(xmlData, xmlFile, owned);
	createdObject->sourceFile = xmlFile->copy();
	if(id) {
		state->script()->setval(id, createdObject);
	}

	xwarn(createdObject, "Warning : could not create an instance of %s", xmlData->Value() );

#ifdef KEEP_AOD_SOURCES
	createdObject->aodSource = xmlData->Clone()->ToElement();
	char ptrValue[64];
	sprintf(ptrValue, "%p", createdObject);
	createdObject->aodSource->SetAttribute("_ptr", ptrValue);
#endif
	return createdObject;
}

// QC:G
ObjectMold* Factory::createMold(TiXmlElement *xmlData, VFS::File* xmlFile)
{
	Factory::moldmapping::iterator it;

	if(!xmlData) {
		return NULL;
	}

	const char* id = xmlData->Attribute("id");

	const char* src = xmlData->Attribute("src");
	if(src) {
		std::map<const char*, const char*> overrides;
		TiXmlAttribute* attr = xmlData->FirstAttribute();
		while(attr) {
			if(strcmp(attr->Name(), "id")!=0 && strcmp(attr->Name(), "src")!=0) {
				overrides[attr->Name()] = attr->Value();
			}
			attr = attr->Next();
		}

		VFS::File* srcFile = VFS::openFile(src, xmlFile);
		ObjectMold* createdObject = loadMold(srcFile, &overrides);
		delete srcFile;

		return createdObject;
	}

	// Find the factory for this class and call it.
	it = moldFactories.find(xmlData->Value());
	xassert(it != moldFactories.end(), "Don't know how to make molds of %s", xmlData->Value());
	xassert(it->second, "NULL mold factory for class %s", xmlData->Value());

	// Call the class-specific factory
	ObjectMold* createdObject = (this->*(it->second))(xmlData, xmlFile);
	createdObject->sourceFile = xmlFile->copy();
	if(id) {
		createdObject->setObjectId(id);
	}

	xwarn(createdObject, "Warning : could not create an instance of %s", xmlData->Value() );

	return createdObject;
}

// QC:A
ScriptableObject* Factory::load(VFS::File* xmlFile, std::map<const char*, const char*>* overrides, ownedset* owned) {
	TiXmlDocument doc(xmlFile->name());
	char* xmlData = xmlFile->readToString();
	if(!xmlData) {
		syslog("Cannot read %s", xmlFile->name());
		return NULL;
	}
	doc.Parse(xmlData);
	AOEFREE(xmlData);

	TiXmlElement* firstElement = doc.FirstChildElement();
	if(!firstElement) {
		return NULL;
	}

	// Override attributes
	if(overrides) {
		for(std::map<const char*, const char*>::iterator i = overrides->begin(); i != overrides->end(); i++) {
			xerror(strcmp(i->first, "id"), "Cannot override id");
			xerror(strcmp(i->first, "src"), "Cannot override src");
			firstElement->SetAttribute(i->first, i->second);
		}
	}

	return createInstance(firstElement, xmlFile, owned);
}

// QC:A
ObjectMold* Factory::loadMold(VFS::File* xmlFile, std::map<const char*, const char*>* overrides) {
	TiXmlDocument doc(xmlFile->name());
	char* xmlData = xmlFile->readToString();
	if(!xmlData) {
		syslog("Cannot read %s", xmlFile->name());
		return NULL;
	}
	doc.Parse(xmlData);
	AOEFREE(xmlData);

	TiXmlElement* firstElement = doc.FirstChildElement();
	if(!firstElement) {
		return NULL;
	}

	// Override attributes
	if(overrides) {
		for(std::map<const char*, const char*>::iterator i = overrides->begin(); i != overrides->end(); i++) {
			assert(strcmp(i->first, "id"));
			assert(strcmp(i->first, "src"));
			firstElement->SetAttribute(i->first, i->second);
		}
	}

	return createMold(firstElement, xmlFile);
}

// QC:G
GraphicalEffects Factory::loadEffects(TiXmlElement *xmlData) {
	GraphicalEffects effects;

	const char* aVal;
	bool b;
	int i;
	coord c;
	Rect r;
	Point p;

	if((aVal=xmlData->Attribute("effects"))) {
		effects.setEffects(aVal);
	}

	if(stringToPoint(xmlData->Attribute("scale"), p))
		effects.setScale(p);
		
	if(stringToPoint(xmlData->Attribute("sectionPointStart"), p))
		effects.setSectionPointStart(p);
	
	if(stringToPoint(xmlData->Attribute("sectionWidthHeight"), p))
		effects.setSectionWidthHeight(p);
	
	if(stringToRect(xmlData->Attribute("section"), r))
		effects.setSection(r);

	if(stringToCoord(xmlData->Attribute("rotation"), &c))
		effects.setRotation(c);

	if(stringToInt(xmlData->Attribute("opacity"), &i))
		effects.setOpacity(i);

	if(stringToInt(xmlData->Attribute("redFilter"), &i))
		effects.setIntegerEffect("RFIL", i);

	if(stringToInt(xmlData->Attribute("greenFilter"), &i))
		effects.setIntegerEffect("GFIL", i);

	if(stringToInt(xmlData->Attribute("blueFilter"), &i))
		effects.setIntegerEffect("BFIL", i);

	effects.setBlendMode(xmlData->Attribute("blend"));

	if(stringToBool(xmlData->Attribute("renderCache"), &b))
		effects.setRenderCache(b);
	
	if((aVal=xmlData->Attribute("preProcess"))) {
		effects.setPreProcess(aVal, NULL);
	}

	if((aVal=xmlData->Attribute("postProcess"))) {
		effects.setPostProcess(aVal, NULL);
	}

	return effects;
}

// QC:?
void Factory::loadAllNodes(VFS::File* xmlFile, std::vector<ScriptableObject*>* destination, ownedset* owned) {
	TiXmlDocument doc(xmlFile->name());
	char* xmlData = xmlFile->readToString();
	if(!xmlData) {
		syslog("Cannot read %s", xmlFile->name());
		return;
	}
	doc.Parse(xmlData);
	AOEFREE(xmlData);

	TiXmlElement* element = doc.FirstChildElement();
	while(element != NULL) {
		syslog("Reading sub element : %s", element->Value());
		destination->push_back(createInstance(element, xmlFile, owned));
		element = element->NextSiblingElement();
	}
}

void Factory::loadNodeMap(VFS::File* xmlFile, std::map<const char*, ScriptableObject*>* destination, ownedset* owned) {
	TiXmlDocument doc(xmlFile->name());
	char* xmlData = xmlFile->readToString();
	if(!xmlData) {
		syslog("Cannot read %s", xmlFile->name());
		return;
	}
	doc.Parse(xmlData);
	AOEFREE(xmlData);

	TiXmlElement* element = doc.FirstChildElement();
	while(element != NULL) {
		syslog("Reading sub element : %s", element->Value());
		const char* eltName = element->Attribute("name");
		syslog("Associated with name : %s", eltName);
		if(eltName && *eltName) (*destination)[eltName] = createInstance(element, xmlFile, owned);
		element = element->NextSiblingElement();
	}
}

void Factory::loadAllNodeMolds(VFS::File* xmlFile, std::list<ObjectMold*>* destination) {
	TiXmlDocument doc(xmlFile->name());
	char* xmlData = xmlFile->readToString();
	if(!xmlData) {
		syslog("Cannot read %s", xmlFile->name());
		return;
	}
	doc.Parse(xmlData);
	AOEFREE(xmlData);

	TiXmlElement* element = doc.FirstChildElement();
	while(element != NULL) {
		syslog("Reading sub element : %s", element->Value());
		destination->push_back(createMold(element, xmlFile));
		element = element->NextSiblingElement();
	}
}

void Factory::loadNodeMapMolds(VFS::File* xmlFile, moldmap* destination) {
	TiXmlDocument doc(xmlFile->name());
	char* xmlData = xmlFile->readToString();
	if(!xmlData) {
		syslog("Cannot read %s", xmlFile->name());
		return;
	}
	doc.Parse(xmlData);
	AOEFREE(xmlData);

	TiXmlElement* element = doc.FirstChildElement();
	while(element != NULL) {
		syslog("Reading sub element : %s", element->Value());
		const char* eltName = element->Attribute("name");
		syslog("Associated with name : %s", eltName);
		if(eltName && *eltName) (*destination)[AOESTRDUP(eltName)] = createMold(element, xmlFile);
		element = element->NextSiblingElement();
	}
}


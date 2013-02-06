#include "scriptable_object.h"
#include <iostream>
#include <string.h>
#ifdef KEEP_AOD_SOURCES
#include "vfs.h"
#include "tinyxml.h"
#endif

// QC:?
ScriptableObject::ScriptableObject(VFS::File* newSourceFile) :
#ifdef KEEP_AOD_SOURCES
	aodSource(NULL),
#endif
		sourceFile(newSourceFile?newSourceFile->copy():NULL)
{}

// QC:?
ScriptableObject::~ScriptableObject() {
#ifdef KEEP_AOD_SOURCES
	if(aodSource)
		delete aodSource;
#endif
	if(sourceFile)
		delete sourceFile;
}

// QC:P
const char* ScriptableObject::getClassName()
{
	return "ScriptableObject";
}

// QC:S
bool ScriptableObject::isInstance(const char* className)
{
	assert(className && *className);
	return (*className == 'S') && strcmp(className, "ScriptableObject") == 0;
}

// QC:P
void ScriptableObject::registerFunctions(Context& v)
{
}

#ifdef KEEP_AOD_SOURCES
// QC:?
static void cleanPtrInAOD(TiXmlElement* elt) {
	elt->RemoveAttribute("_ptr");
	
	for( TiXmlElement* child = elt->FirstChildElement(); child; child = child->NextSiblingElement() )
		cleanPtrInAOD(child);
}

// QC:?
void ScriptableObject::saveAOD(VFS::File* file)
{
	TiXmlDocument* doc = new TiXmlDocument();
	TiXmlElement* xml = aod()->Clone()->ToElement();

	cleanPtrInAOD(xml);
	doc->LinkEndChild(xml);

	TiXmlPrinter printer;
	printer.SetStreamPrinting();
	doc->Accept(&printer);

	file->truncate();
	file->write((char*)printer.CStr(), printer.Size());

	delete doc;
}
#endif

#include <string.h>
#include <time.h>
#include <math.h>
#include "script_monitor.h"
#include "context.h"
#include "vfs_file.h"
#include "aurora_engine.h"
#include "scripted_function.h"
#define HEADER script_monitor
#define HELPER scriptable_object_cpp
#include "include_helper.h"

void ScriptMonitor::print(const char* msg)
{
#ifdef SYSLOG
	syslog("%s", msg);
#else
#ifdef SCRIPTLOG
	std::cout << "ScriptMonitor:\"" << msg << "\"" << std::endl;
#else
	std::cout << msg << std::endl;
#endif /* SCRIPTLOG */
#endif /* SYSLOG */
}

int ScriptMonitor::time() {
	return getTime();
}

// Trigonometric functions for SSS

int ScriptMonitor::cos(int angle) {
	return (int)(1000*cosf((float)angle)*M_PI/180.0);
}

int ScriptMonitor::sin(int angle) {
	return (int)(1000*sinf((float)angle)*M_PI/180.0);
}

int ScriptMonitor::tan(int angle) {
	return (int)(1000*tanf((float)angle)*M_PI/180.0);
}

int ScriptMonitor::acos(int val) {
	return (int)(180.0*acosf((float)val/1000)/M_PI);
}

int ScriptMonitor::asin(int val) {
	return (int)(180.0*asinf((float)val/1000)/M_PI);
}

int ScriptMonitor::atan(int val) {
	return (int)(180.0*atanf((float)val/1000)/M_PI);
}

int ScriptMonitor::atn2(int valX, int valY) {
	return (int)(180.0*atan2((float)valX/1000, (float)valY/1000)/M_PI);
}

bool ScriptMonitor::wait(bool event) {
	return event;
}

bool ScriptMonitor::waitFor(ScriptableObject* blocker) {
	if(!blocker)
		return false;

	Renderable* r = dynamic_cast<Renderable*>(blocker);
	ScriptedFunction* f = NULL;
	Blocking* b = NULL;
	if(!r) {
		f = dynamic_cast<ScriptedFunction*>(blocker);
		if(f)
			b = dynamic_cast<Blocking*>(f);
		else
			b = dynamic_cast<Blocking*>(blocker);
	} else 
		b = dynamic_cast<Blocking*>(r);

	xassert(b, "Dynamic cast problem in waitFor.");

	return !b || !b->isFinished();
}

TiXmlNode* ScriptMonitor::xml(const char* data) {
	TiXmlDocument doc;
	if(data)
		doc.Parse(data);
	if(!doc.RootElement())
		return new TiXmlElement("");
	return doc.RootElement()->Clone();
}

int ScriptMonitor::xmlChildCount(TiXmlNode* node) {
	int i = 0;
	for(node = node->FirstChild(); node; node = node->NextSibling())
		i++;
	return i;
}

void ScriptMonitor::delobj(ScriptableObject* obj) {
	xerror(obj, "Cannot delete NULL pointer. Maybe the variable is not an object.");
	delete obj;
}

void ScriptMonitor::delxml(TiXmlNode* node) {
	xerror(node, "Cannot delete NULL pointer. Maybe the variable is not an object.");
	delete node;
}

// QC:?
VfsFile* ScriptMonitor::open(const char* filename, ScriptableObject* pathRoot) {
	xerror(pathRoot, "No directory root.");
	return new VfsFile(pathRoot->engine(), filename, pathRoot->source());
}

/* La classe Script Monitor permet de declarer des methodes globales facilement
 * Une instance doit toujours etre chargee dans un Contexte.
 * C'est aussi basique que <iostream> en C++ !
 */

#ifndef __script_monitor__
#define __script_monitor__
class ScriptMonitor;
class ScriptableObject;

#include <iostream>
#include "abstract.h"
#include "state.h"

/** ScriptMonitor is the mandatory class for AOS.
At least one instance must be added to each Context, to provide some useful global functions.
*/
class ScriptMonitor : public Abstract, public ScriptableObject
{
private:

public:
#define HEADER script_monitor
#define HELPER scriptable_object_h
#include "include_helper.h"
	ScriptMonitor() {}
	ScriptMonitor(AuroraEngine* parentEngine) : Abstract(parentEngine) {}

	virtual ~ScriptMonitor() {
	}

	/** Prints a message to the standard output.
	@param msg the message to print.
	*/
	static void print(const char* msg);

	/** Get system time in milliseconds.
	This function returns the system time in milliseconds.<p>
	Millisecond granularity is not guaranteed.
	@return Returns a value representing system time in milliseconds.
	*/
	static int time();

	/** Block until the parameter becomes true.
	 * @return true if the parameter is true, false otherwise.
	 */
	static bool wait(bool event);

	/** Wait until a blocking object is unblocked.
	 */
	static bool waitFor(ScriptableObject* blocker);

	/* Trigonometric functions. Angles are in degrees, values are multiplied by 1000. */
	static int cos(int angle);
	static int sin(int angle);
	static int tan(int angle);
	static int acos(int value);
	static int asin(int value);
	static int atan(int value);
	static int atn2(int valX, int valY);

	static int len(int value) { return value; }

	/* File functions */
	static VfsFile* open(const char* filename, ScriptableObject* pathRoot);

	/* Conversion functions */
	static int integer(const char* value) { return atoi(value); }
	static char* string(char* value) { return value; }
	static TiXmlNode* xml(const char* value);

	/* XML functions */
	static TiXmlNode* xmlElement(const char* name) { return new TiXmlElement(name); }
	static TiXmlNode* xmlText(const char* text) { return new TiXmlText(text); }
	static TiXmlNode* xmlComment(const char* comment) { return new TiXmlComment(comment); }
	static int xmlChildCount(TiXmlNode* node);

	/* Delete operators. */
	static void delobj(ScriptableObject* obj);
	static void delxml(TiXmlNode* node);

	/* The final solution */
	static void quitEngine() {
#ifdef WIN32
		TerminateProcess(GetCurrentProcess(), 0);
		// The legend will never die
#else
		exit(0);
#endif
	}
};

#endif


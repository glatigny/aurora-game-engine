#ifndef CONTEXT_H
#define CONTEXT_H

class Context;
class ScriptVariable;
class ScriptedFunction;
class ContextInitializer;

#include "scriptable_object.h"
#include "abstract.h"
#include "translatable.h"
#include "point.h"
#include "script_variable.h"

// Taille maximale d'un nom de variable.
#define MAXNAMELEN 80

#include <cstdarg>
#include <vector>
#include <list>

class VariableList
{
private:
	char name[MAXNAMELEN];
	ScriptVariable* v;
	VariableList* next;

public:
	VariableList();
	VariableList(const VariableList& v);
	~VariableList();

	ScriptVariable* getVar(const char* name);

	void getVarlist(ScriptVariable& v, const bool hidden);

	template <class T>
		void getObjectList(std::vector<T>* destination, const char* className);

	void setval(Context* sc, const char* name, ScriptVariable& var);
	ScriptVariable* newVar(Context* sc, const char* vName);
	void flush(); // Vide les variables nulles
	void reset();
};

class ClassList
{
private:
	const char* className; // Les noms de classes n'ont pas besoin d'etre copies puisqu'ils sont constants
	ClassList* next;
public:
	ClassList();
	~ClassList();

	bool isRegistered(const char* name);
	bool registerClass(const char* name); // Retourne vrai si la classe a ete ajoutee, faux si elle existait deja
	void unregisterClass(const char* name);
};

class Context : public Abstract
{
	friend class ContextInitializer;
private:
	/** Supercontext.
	Variables inside the superContext can be accessed, but no new variable can be added to it.
	It respects inheritance of variables like C variable scopes.
	*/
	Context* super;
	VariableList vlist[27];
	ClassList registeredClasses[27];

	/** Main execution thread.
	 */
	ScriptedFunction* mainThread;

	/** Set the main thread of this context.
	 * The main thread is the one which is executed when framestep() is called.
	 */
	void setMainThread(ScriptedFunction* newMainThread);

	/** Returns the first letter of 'name'.
	The first letter is returned as a number between 0 and 25.
	The function returns 26 if the name does not start with a letter.
	@param name the name of the variable
	@return First letter (0-25) or 26 if not a letter
	*/
	int hash(const char* name);

	/** Tests wether a class is registered
	 * @param name the class name to test for registration.
	 * @return true if the class is registered, false otherwise.
	 */
	bool isRegistered(const char* name);

	/** Calls a function stored in a ScriptVariable with already parsed parameters.
	 */
	ScriptVariable callFunctionWithParams(ScriptVariable* fvar, Context* params);

public:
	//Context();
	Context(AuroraEngine* parentEngine);
	Context(Context& c);
	~Context();

#ifdef GCLOG
	static void listContexts();
#endif

	Context& operator=(const Context& c);

	ScriptVariable* operator[](const char* name) {
		return getVar(name);
	}

	void reset();

	const char* getClassName() { return "Context"; }

	ScriptVariable* getVar(const char* name);
	bool getbool(const char* name);
	int getint(const char* name);
	int getintPercent(const char* name);
	int* getintptr(const char* name);
	coord getcoord(const char* name) { return (coord)getint(name); } // TODO : implement float and correct coordinates in the script engine.
	dur getdur(const char* name) { return (dur)getint(name); }
	char* getstr(const char* name);
	char* getdata(const char* name);
	char** getdataptr(const char* name);
	char* getparams(const char* name);
	char** getparamsptr(const char* name);
	char* getfct(const char* name);
	ScriptableObject* getobj(const char* name);
	char* getclass(const char* name);
	Context* gethashtable(const char* name);
	ScriptTable* gettable(const char* name);
	Point getPoint(const char* name);
	Point getPointPercent(const char* name);
	Translatable* getTranslatable(const char* name);
	Rect getRect(const char* name);
	Context* getsuper();

	/** Returns a space-separated list of all the variables stored in this context.
	The list is returned in a variable of type string.
	@param v the variable that will store the list.
	@param hidden if true, hidden variables will be listed (hidden variables begin with @ and are for internal use only).
	*/
	void getVarlist(ScriptVariable& v, const bool hidden);

	/** Returns a vector of objects of a specific class.
	 * This functions populates the vector with objects from this context. The class of listed objects must be specified as a string.
	 * @param destination the vector which will hold objects.
	 * @param className the class of objects to put into the vector. The class must be the same as the template parameter.
	 */
	template <class T>
		void getObjectList(std::vector<T>* destination, const char* className);

	ScriptVariable* registerScriptVariable(const char* name); // Cree une variable locale, meme si une variable de meme nom est dans le superContexte
	void resetval(const char* name);
	void setval(const char* name, int val);
	void setval(const char* name, int x, int y);
	void setval(const char* name, Point p);
	void setval(const char* name, char* str);
	void setval(const char* name, ScriptableObject* o); // Objet scriptable
	void setval(const char* name, char* dataptr, char* paramsptr, int value); // Pointeurs bruts
	void setval(const char* name, ScriptVariable& var); // Recopie de ScriptVariable
	void setval(const char* name, ScriptTable* table);
	void setsuper(Context* superContext);

	/** Destroy unused variables.
	FIXME : Still unimplemented.
	Is it really useful ? Contexts usually have a short lifespan. Objects are self-destroyed by another mechanism and do not need flush() !
	*/
	void flush();

	/** Register a new class in the context.
	Registering a class makes its methods available.
	Classes are automatically registered when objects are added into the context. If you absolutely need to register a class without adding any object to the Context (very rare case), use this function.
	@param o an instance of the class to register.
	*/
	void registerClass(ScriptableObject &o);

	/** Register all known native classes.
	 */
	void registerAllNativeClasses();

	/** (For internal use only) Register a function in the context.
	Called automatically when registerClass is called.
	@see registerClass
	*/
	void register_function(void* callback_ptr, const char* classname, const char* name, const char* params, const int flags, void* block_callback_ptr);

	/** (For internal use only) Register a ScriptThread in the execution list.
	*/
	void registerScriptThread(ScriptedFunction* t);

	/** (For internal use only) Unregister a ScriptThread from the execution list.
	 */
	void unregisterScriptThread(ScriptedFunction* t);

	/** Run scripts for one frame.
	All threads will run for one frame.
	@see threadsRunning
	*/
	void framestep();

	/** Tells if some threads are still running.
	It does not tell if the context is in a blocked state, use isRunning for this.
	@return true if some subThreads still run, false otherwise.
	@see framestep
	@see isRunning
	*/
	bool threadsRunning();

	/** Tells if the main thread is still running.
	@return true if some threads still need more frames to finish, false otherwise.
	@see framestep
	@see threadsRunning
	*/
	bool isRunning();

	/** (For internal use only) Tries to stop all threads and deletes the Context if successful.
	@throw itself : execution was interrupted by a blocking call.
	*/
	void kill();

	/** Calls a scripted member function.
	This is the right way to call a member function of a scriptable_object.<p>
	If the function has been redefined by a script, the scripted version will be called.
	Otherwise, the native version will be called.<p>
	Parameters will be interpreted as ints.
	@param fobj the object on which the method will be called.
	@param fname the name of the function.
	@param ... the parameters to be passed to the function. All parameters must be integers.
	@return nothing. Return value of the function is discarded.
	*/
	ScriptVariable callFunction(ScriptableObject& fobj, const char* fname, ...);

	/** Calls a scripted static function.
	This is the right way to call a static function.<p>
	If the function has been redefined by a script, the scripted version will be called.
	Otherwise, the native version will be called.<p>
	Parameters will be interpreted as ints.
	@param fname the name of the function.
	@param ... the parameters to be passed to the function. All parameters must be integers.
	@return nothing. Return value of the function is discarded.
	*/
	ScriptVariable callFunction(char* fname, ...);

	/** Calls a scripted member function with any type of parameter.
	This is the right way to call a member function of a scriptable_object.<p>
	If the function has been redefined by a script, the scripted version will be called.
	Otherwise, the native version will be called.<p>
	Parameters will be interpreted as given in the format string. The format string is a
	null-terminated table of char with the following values :
	<ul><li>i = int</li>
	    <li>s = string (char*)</li>
	    <li>o = object (ScriptableObject*)</li>
	</ul>
	@param ptype list of parameter types.
	@param fobj the object on which the method will be called.
	@param fname the name of the function.
	@param ... the parameters to be passed to the function. All parameters must be integers.
	@return nothing. Return value of the function is discarded.
	*/
	ScriptVariable callFunction(ScriptableObject& fobj, char* fname, const char* ptype, ...);

	/** Calls a scripted static function with any type of parameter.
	This is the right way to call a static function.<p>
	If the function has been redefined by a script, the scripted version will be called.
	Otherwise, the native version will be called.<p>
	Parameters will be interpreted as given in the format string. The format string is a
	null-terminated table of char with the following values :
	<ul><li>i = int</li>
	    <li>s = string (char*)</li>
	    <li>o = object (ScriptableObject*)</li>
	</ul>
	@param ptype list of parameter types.
	@param fname the name of the function.
	@param ... the parameters to be passed to the function. All parameters must be integers.
	@return nothing. Return value of the function is discarded.
	*/
	ScriptVariable callFunction(char* fname, const char* ptype, ...);

	/** Load and run a script from a string.
	Stores the code into the Context and executes it.
	Code is duplicated, so the buffer may be destroyed after this call.<p>
	All variables defined in the code will be global.<p>
	@param code the zero-terminated buffer with SSS code into it.
	@see load_script_from_file
	*/
	void load_script(VFS::File* sourceFile, char* code);

	/* AOEObject */

	VFS::File* source() { return NULL; }
};

#endif


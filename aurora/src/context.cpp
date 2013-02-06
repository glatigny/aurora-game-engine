#include "context.h"
#include "context_initializer.h"
#include "scripted_function.h"
#include "script_map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <iostream>

/***********************************************/


// class VariableList

// QC:P
VariableList::VariableList()
{
	name[0] = '\0';
	v = NULL;
	next = NULL;
}

// QC:W
VariableList::VariableList(const VariableList& v)
{
	// Recopie de tous les membres

	// TODO
}

// QC:L
VariableList::~VariableList()
{
	reset();
}

// QC:?
void VariableList::reset()
{
	ScriptVariable* vtmp = v;
	VariableList* ntmp = next;

	v = NULL;
	next = NULL;

	if(vtmp != NULL)
	{
		delete vtmp;
	}

	if(ntmp != NULL)
	{
		delete ntmp;
	}
}

// QC:G
ScriptVariable* VariableList::getVar(const char* vName)
{
	assert(strlen(vName) >= 1);
	for(VariableList* vl = this; vl != NULL; vl = vl->next) {
		assert(vl);
		assert(vl->name);
		assert(*(vl->name) || !*(vl->name));
	}

	// Comme les noms sont dans l'ordre croissant, arret de la verif si on est deja trop loin
	for(VariableList* vl = this; vl != NULL && *vl->name && vl->name[1]<=vName[1]; vl = vl->next)
	{
		if(strcmp(vl->name, vName) == 0)
		{
			return vl->v;
		}
	}

	return NULL;
}

// QC:G
void VariableList::getVarlist(ScriptVariable& v, const bool hidden) {
	for(VariableList* vl = this; vl != NULL && *vl->name; vl = vl->next)
	{
		if(hidden || *vl->name != '@')
		{
			if(*v.getstr())
			{
				v.concat(" ");
			}
			v.concat(vl->name);
		}
	}
}

// QC:U (utilisee dans le moteur de script, mais pas vraiment testee. indirectement Leaking)
template <class T> void VariableList::getObjectList(std::vector<T>* destination, const char* className) {
	assert(className);
	assert(destination);

	for(VariableList* vl = this; vl != NULL && *vl->name; vl = vl->next)
	{
		if(*vl->name != '@' && vl->v->gettype() == VAR_OBJ && vl->v->getobj()->isInstance(className))
		{
			destination->push_back((T)(vl->v->getobj()));
		}
	}
}

// Cree une nouvelle variable si elle n'existe pas et retourne le pointeur vers la variable
// QC:G
ScriptVariable* VariableList::newVar(Context* sc, const char* vName)
{
	assert(strlen(vName) >= 1);

	if(v == NULL)
	{
		// La liste n'est pas initialisee
		strncpy(name, vName, MAXNAMELEN);
		v = new ScriptVariable();
		return v;
	}

	VariableList* vl;
	VariableList* cvl = this;

	// Boucle tant que le deuxieme caractere est inferieur au nom recherche
	for(vl = this; vl != NULL && *vl->name && vl->name[1]<=vName[1]; vl = vl->next)
	{
		if(strcmp(vl->name, vName) == 0)
		{
			// La variable existe deja
			return vl->v;
		}

		cvl = vl;
	}

	// La variable n'existe pas, on va la creer
	VariableList* nvl = new VariableList;
	ScriptVariable* nvar = new ScriptVariable();

	if(vl == this)
	{
		// Le nouvel element doit etre place en premiere position

		// Deplace le premier element actuel dans nvl
		strncpy(nvl->name, cvl->name, MAXNAMELEN);
		nvl->v = cvl->v;
		nvl->next = cvl->next;

		// Met la nouvelle variable dans le premier element
		strncpy(cvl->name, vName, MAXNAMELEN);
		cvl->v = nvar;

		// Chaine le premier element sur l'ancien premier element
		cvl->next = nvl;
	}
	else
	{
		// Le nouvel element doit etre place apres cvl
		nvl->v = nvar;
		nvl->next = cvl->next;
		strncpy(nvl->name, vName, MAXNAMELEN);
		cvl->next = nvl;
	}

	return nvar;
}

// QC:G
void VariableList::setval(Context* sc, const char* vName, ScriptVariable& var)
{
	ScriptVariable* v = newVar(sc, vName);
	v->setval(var);
}


/***********************************************/
// classlist (a remplacer par un vector ?)

// QC:P
ClassList::ClassList()
{
	className = NULL;
	next = NULL;
}

// QC:F (les noms de classes ne sont pas liberes car consideres comme constants)
ClassList::~ClassList()
{
	if(next != NULL) delete next;
}

// QC:F (comparaison de chaines avec == )
bool ClassList::isRegistered(const char* name)
{
	ClassList* l;

	assert(name);

	for(l = this; l != NULL; l=l->next)
	{
		if(name == l->className)
			return true; // La classe existe
	}

	return false;
}

// QC:F (comparaison de chaines avec == )
bool ClassList::registerClass(const char* name)
{
	ClassList* newl;

	assert(name);

	if(className != NULL)
	{
		ClassList* l = this;
		while(l->next != NULL)
		{
			if(name == l->className)
				return false; // Pas besoin d'enregistrer la classe

			l = l->next;
		}
		if(name == l->className)
			return false; // Pas besoin d'enregistrer la classe

		newl = new ClassList;
		l->next = newl;
	}
	else
	{
		newl = this;
	}

	// Classe non trouvee, on l'enregistre
	newl->className = name;

	// Besoin d'enregistrer la classe
	return true;
}

// QC:F (comparaison de chaines avec == )
void ClassList::unregisterClass(const char* name)
{
	assert(name);

	if(next == NULL)
	{
		if(name == className)
		{
			className = NULL;
		}
		return;
	}

	if(name == className) {
		ClassList* lnext = next;
		className = next->className;
		next = next->next;
		delete lnext;
		return;
	}

	for(ClassList* l = this; l->next != NULL; l = l->next)
	{
		if(name == l->next->className)
		{
			ClassList* lnext = l->next;
			if(l->next->next != NULL)
			{
				l->className = lnext->className;
				l->next = (lnext != NULL)?lnext->next:NULL;

				lnext->next = NULL;
			}
			delete lnext;
			return;
		}
	}
}

/***********************************************/
/* context                                     */
/***********************************************/

// class context

// QC:P
Context::Context(AuroraEngine* parentEngine) : Abstract(parentEngine), super(NULL), mainThread(NULL)
{
}

// QC:W (VariableList.operator= non implemente)
Context::Context(Context& c) : Abstract(c.engine())
{
	abort(); // TODO : implementation needed
	mainThread = NULL;
	setsuper(c.super);
	for(int i=0; i<27; i++)
	{
		vlist[i] = c.vlist[i];
	}
}

Context::~Context()
{
	if(mainThread)
		delete mainThread;
	reset();
}

// QC:?
void Context::reset() {
	for(int i=0; i<27; i++)
	{
		vlist[i].reset();
	}
}

// QC:P
ScriptVariable* Context::getVar(const char* name)
{
	assert(name);

	ScriptVariable* v;

	// Recherche la ScriptVariable dans le Contexte actuel
	v = vlist[hash(name)].getVar(name);

	// Recherche dans le superContexte si le Contexte actuel a echoue
	if(v == NULL && super != NULL)
	{
		v = super->getVar((char*)name);
	}

	return v;
}

// Implementation de getbool(), getint(), getstr(), ...

// QC:P
Context* Context::getsuper()
{
	return super;
}

// QC:P
#define GET(scripttype, defaultvalue) \
Context::get ## scripttype(const char* name) \
{ \
	assert(name); \
	ScriptVariable* v = vlist[hash(name)].getVar(name); \
	if(v==NULL && super!=NULL) \
		return super->get ## scripttype(name); \
	return v==NULL?(defaultvalue):v->get ## scripttype(); \
}
	bool GET(bool, false)
	int GET(int, 0)
	int GET(intPercent, 0)
	int* GET(intptr, NULL)
	char* GET(str, NULL)
	char* GET(data, NULL)
	char** GET(dataptr, NULL)
	char* GET(params, NULL)
	char** GET(paramsptr, NULL)
	ScriptableObject* GET(obj, NULL)
	Context* GET(hashtable, NULL)
	ScriptTable* GET(table, NULL)
	char* GET(class, NULL)
#undef GET
// Fin de l'implementation de get<type>

// QC:P
Point Context::getPoint(const char* name)
{
	assert(name);

	Point p;
	ScriptVariable* v = vlist[hash(name)].getVar(name);
	if(v==NULL && super!=NULL)
		return super->getPoint(name);
	return v==NULL?(p):v->getPoint();
}

// QC:P
Point Context::getPointPercent(const char* name)
{
	assert(name);

	Point p;
	ScriptVariable* v = vlist[hash(name)].getVar(name);
	if(v==NULL && super!=NULL)
		return super->getPointPercent(name);
	return v==NULL?(p):v->getPointPercent();
}

// QC:A (le tampon statique rend la fonction non reentrante)
Translatable* Context::getTranslatable(const char* name)
{
	assert(name);

	static Point p;
	p = getPoint(name);
	return &p;
}

// QC:P
Rect Context::getRect(const char* name)
{
	assert(name);

	ScriptVariable* v = vlist[hash(name)].getVar(name);
	if(v==NULL && super!=NULL)
		return super->getRect(name);
	return v==NULL?(Rect()):v->getRect();
}

// QC:U
void Context::getVarlist(ScriptVariable& v, const bool hidden) {
	v.setval("");
	for(int i=0; i<27; i++) {
		vlist[i].getVarlist(v, hidden);
	}
}

// QC:U
template <class T> void Context::getObjectList(std::vector<T>* destination, const char* className) {
	for(int i=0; i<27; i++) {
		vlist[i].getObjectList<T>(destination, className);
	}
}

// QC:G
void Context::resetval(const char* name)
{
	assert(name);

	// Essaie de remplacer la variable locale
	ScriptVariable* v = getVar(name);
	if(v == NULL)
	{
		v = vlist[hash(name)].newVar(this, name);
	}
	v->setval();
}

// QC:G
#define SET(type) void Context::setval(const char* name, type val) \
{ \
	assert(name); \
	ScriptVariable* v = getVar(name); \
	if(v == NULL) { \
		v = vlist[hash(name)].newVar(this, name); \
	} \
	v->setval(val); \
}
	SET(int)
	SET(char*)
#undef SET

// QC:P
void Context::setval(const char* name, int x, int y)
{
	assert(name);

	// Essaie de remplacer la variable locale
	ScriptVariable* v = getVar(name);
	if(v == NULL)
	{
		v = vlist[hash(name)].newVar(this, name);
	}
	v->setval(x, y);
}

// QC:?
void Context::setval(const char* name, Point p)
{
	assert(name);

	// Essaie de remplacer la variable locale
	ScriptVariable* v = getVar(name);
	if(v == NULL)
	{
		v = vlist[hash(name)].newVar(this, name);
	}
	v->setval(p.getX(), p.getY());
}

// QC:?
void Context::setval(const char* name, ScriptTable* table)
{
	assert(name);

	// Essaie de remplacer la variable locale
	ScriptVariable* v = getVar(name);
	if(v == NULL)
	{
		v = vlist[hash(name)].newVar(this, name);
	}
	v->setval(table);
}

// QC:G
void Context::setval(const char* name, ScriptableObject* o)
{
	assert(name);

	if(o)
	{
		// Essaie de remplacer la variable locale
		ScriptVariable* v = getVar(name);
		if(v == NULL)
		{
			v = vlist[hash(name)].newVar(this, name);
		}
		v->setval(o);
	}
	else
	{
		resetval(name);
	}
}

// QC:A (hack : les variables raw sont des hacks, mais cela fonctionne bien !)
void Context::setval(const char* name, char* dataptr, char* paramsptr, int value)
{
	assert(name);

	// Essaie de remplacer la variable locale
	ScriptVariable* v = getVar(name);
	if(v == NULL)
	{
		v = vlist[hash(name)].newVar(this, name);
	}
	v->setval(dataptr, paramsptr, value);
}


// QC:G
void Context::setval(const char* name, ScriptVariable& var)
{
	assert(name);

	// Essaie de remplacer la variable locale
	ScriptVariable* v = getVar(name);
	if(v == NULL)
	{
		v = vlist[hash(name)].newVar(this, name);
	}
	v->setval(var);
}

// QC:G
void Context::setsuper(Context* superContext)
{
#ifndef NDEBUG
	// Assert that the superContext relationship has no known loops.
	Context* s = superContext;
	while(s) {
		assert(s != this);
		s = s->super;
	}
#endif

	super = superContext;
}

// QC:S
int Context::hash(const char* name)
{
	assert(name);

#ifdef SCRIPTLOG
	return 26; // Easier for debug
#else
	if(*name>='a' && *name<='z')
	{
		return (int)(*name-'a');
	}
	if(*name>='A' && *name<='Z')
	{
		return (int)(*name-'A');
	}

	return 26;
#endif
}

// QC:P
ScriptVariable* Context::registerScriptVariable(const char* name)
{
	assert(name);

	ScriptVariable* v = vlist[hash(name)].newVar(this, name);
	v->setval();

	return v;
}

// QC:P
void Context::registerAllNativeClasses()
{
	ContextInitializer::registerAllNativeClasses(this);
}

// QC:P
void Context::registerClass(ScriptableObject& o)
{
	const char* className = o.getClassName();
	assert(className);

	if(!isRegistered(className))
	{
		registeredClasses[hash(className)].registerClass(className);
		o.registerFunctions(*this);
	}
}

// QC:P
inline bool Context::isRegistered(const char* className)
{
	assert(className);

	return registeredClasses[hash(className)].isRegistered(className)
		|| (super != NULL && super->isRegistered(className));
}

// QC:S (XXX : buffer fname taille statique, appels repetes a strncat)
void Context::register_function(void* callback_ptr, const char* classname, const char* name, const char* params, const int flags, void* block_callback_ptr)
{
	assert(callback_ptr);
	assert(classname && *classname);
	assert(name && *name);
	assert(params && *params);
	// we need to assert params are well formed, but is it useful ?

	char fname[MAXNAMELEN];

	if((flags & FCTFLAG_STATIC) == FCTFLAG_STATIC)
	{
		// Fonction globale. Elle s'appelle par son nom seul.
		strncpy(fname, name, MAXNAMELEN - 1);
	}
	else
	{
		// Fonction non globale. elle doit etre precedee du nom de classe et d'un point.
		strncpy(fname, classname, MAXNAMELEN - 1);
		strncat(fname, ".", MAXNAMELEN - 1);
		strncat(fname, name, MAXNAMELEN - 1);
	}
	fname[MAXNAMELEN - 1] = '\0';

	ScriptVariable* f = vlist[hash(fname)].newVar(this, fname);
	f->setfct((char*)callback_ptr, params, flags, (char*)block_callback_ptr);
}

// QC:G
void Context::framestep()
{
	if(mainThread)
		mainThread->framestep();
}

// QC:P
bool Context::threadsRunning()
{
	if(mainThread)
		return mainThread->threadsRunning();

	return false;
}

// QC:?
bool Context::isRunning()
{
	if(mainThread)
		return mainThread->isRunning();

	return false;
}

// QC:?
ScriptVariable Context::callFunctionWithParams(ScriptVariable* fvar, Context* params) {
	if(fvar->getint() & FCTFLAG_BLOCKING)
	{
		// La fonction est bloquante
		static const char* emptyString = "";
		ScriptedFunction* nt = new ScriptedFunction(engine(), source(), emptyString, params);

		if(fvar->gettype() == VAR_FCT)
		{
			nt->setInstr("fct");
			nt->instr_data.instr_fct.fctVar = fvar;
			nt->instr_data.instr_fct.fctParams = params;
			nt->instr_data.instr_fct.fctPtr = (char*)emptyString;

			if(mainThread) {
				mainThread->addThread(nt);
			} else {
				mainThread = nt;
			}

			nt->framestep();
		}
		else if(fvar->gettype() == VAR_TXTFCT)
		{
			// TODO :  support classes
			nt->setInstr("txtfct");
			params->setsuper(this);
			nt->instr_data.instr_fct.fctPtr = (char*)emptyString;

			nt->subFunctionRun(fvar->getstr(), true, params);
		}
	}
	else
	{
		ScriptVariable retVal;

		// La fonction n'est pas bloquante
		// On l'appelle tout simplement
		params->setsuper(this);
		fvar->callNativeFunction(params);
		retVal = *(params->getVar("@retval"));
		delete params;
		return retVal;
	}

	return ScriptVariable(); // No return value
}

// QC:L (leaks non verifies)
ScriptVariable Context::callFunction(ScriptableObject& fobj, const char* shortfname, ...)
{
	//XXX : Necessary ? registerClass(fobj);

	char fname[MAXNAMELEN]="";

	// Calcule le nom de la fonction
	strcat(fname, fobj.getClassName());
	strcat(fname, ".");
	strcat(fname, shortfname);

	Context* params = new Context(engine());

	// Sauvegarde le "this"
	params->setval("this", &fobj);

	// Positionne le topContext
	params->setval("@tc", (char*)this, NULL, 0);

	// Recupere la fonction
	ScriptVariable* fvar = getVar(fname);
	if(fvar == NULL || (fvar->gettype() != VAR_FCT && fvar->gettype() != VAR_TXTFCT))
	{
		// La fonction n'existe pas
		return ScriptVariable();
	}

	// Lecture des parametres
	va_list pv; // Parameter Values
	va_start(pv, shortfname);

	// Interprete les variables voulues par la fonction
	char* wantedParams = fvar->getparams();
	if(*wantedParams != '(')
	{
		delete params;
		return ScriptVariable();
	}

	ScriptedFunction::str_skipblanks(wantedParams);

	while(*wantedParams && *wantedParams != ')')
	{
		// Saute le separateur
		wantedParams++;
		ScriptedFunction::str_skipblanks(wantedParams);
		if(*wantedParams == '&') {
			// Skip address indicator
			wantedParams++;
		}

		// Lit le nom du parametre
		char* left = wantedParams;
		ScriptedFunction::str_skipvarname(wantedParams);
		char* pName = (char*)alloca(wantedParams - left + 1);
		memcpy(pName, left, wantedParams - left);
		pName[wantedParams - left] = '\0';
		ScriptedFunction::str_skipblanks(wantedParams);

		// pName contient le nom du parametre

		if(*wantedParams!=',' && *wantedParams != ')')
		{
			syslog("Wrong number of parameters while calling function.");
			AOEFREE(pName);
			delete params;
			return ScriptVariable();
		}

		// Associe le nom du parametre a sa valeur
		params->setval(pName, va_arg(pv, int));
	}

	va_end(pv);
	// Lecture des parametres terminee

	return callFunctionWithParams(fvar, params);
}

// QC:L (leaking non verifie)
ScriptVariable Context::callFunction(char* fname, ...)
{
	Context* params = new Context(engine());

	// Positionne le topContext
	params->setval("@tc", (char*)this, NULL, 0);

	// Recupere la fonction
	ScriptVariable* fvar = getVar(fname);
	if(fvar == NULL || (fvar->gettype() != VAR_FCT && fvar->gettype() != VAR_TXTFCT))
	{
		// La fonction n'existe pas
		return ScriptVariable();
	}

	// Lecture des parametres
	va_list pv; // Parameter Values
	va_start(pv, fname);

	// Interprete les variables voulues par la fonction
	char* wantedParams = fvar->getparams();
	if(*wantedParams != '(')
	{
		delete params;
		return ScriptVariable();
	}

	ScriptedFunction::str_skipblanks(wantedParams);

	while(*wantedParams && *wantedParams != ')')
	{
		// Saute le separateur
		wantedParams++;
		ScriptedFunction::str_skipblanks(wantedParams);

		// Lit le nom du parametre
		char* left = wantedParams;
		ScriptedFunction::str_skipvarname(wantedParams);
		char* pName = (char*)alloca(wantedParams - left + 1);
		memcpy(pName, left, wantedParams - left);
		pName[wantedParams - left] = '\0';
		ScriptedFunction::str_skipblanks(wantedParams);

		// pName contient le nom du parametre

		if(*wantedParams!=',' && *wantedParams != ')')
		{
			syslog("Wrong number of parameters while calling function.");
			AOEFREE(pName);
			delete params;
			return ScriptVariable();
		}

		// Associe le nom du parametre a sa valeur
		params->setval(pName, va_arg(pv, int));
	}

	va_end(pv);
	// Lecture des parametres terminee

	return callFunctionWithParams(fvar, params);
}

// QC:L (leaking non verifie)
ScriptVariable Context::callFunction(ScriptableObject& fobj, char* shortfname, const char* ptype, ...)
{
	//XXX : necessary ? registerClass(fobj);

	char fname[MAXNAMELEN]="";

	// Calcule le nom de la fonction
	strcat(fname, fobj.getClassName());
	strcat(fname, ".");
	strcat(fname, shortfname);

	Context* params = new Context(engine());

	// Sauvegarde le "this"
	params->setval("this", &fobj);

	// Positionne le topContext
	params->setval("@tc", (char*)this, NULL, 0);

	// Recupere la fonction
	ScriptVariable* fvar = getVar(fname);
	if(fvar == NULL || (fvar->gettype() != VAR_FCT && fvar->gettype() != VAR_TXTFCT))
	{
		// La fonction n'existe pas
		delete params;
		return ScriptVariable();
	}

	// Lecture des parametres
	va_list pv; // Parameter Values
	va_start(pv, ptype);

	// Interprete les variables voulues par la fonction
	char* wantedParams = fvar->getparams();
	if(*wantedParams != '(')
	{
		delete params;
		return ScriptVariable();
	}

	ScriptedFunction::str_skipblanks(wantedParams);

	int pcount=0;
	while(*wantedParams && *wantedParams != ')')
	{
		// Saute le separateur
		wantedParams++;
		ScriptedFunction::str_skipblanks(wantedParams);

		// Lit le nom du parametre
		char* left = wantedParams;
		ScriptedFunction::str_skipvarname(wantedParams);
		char* pName = (char*)AOEMALLOC(wantedParams - left + 1);
		memcpy(pName, left, wantedParams - left);
		pName[wantedParams - left] = '\0';
		ScriptedFunction::str_skipblanks(wantedParams);

		// pName contient le nom du parametre

		if(*wantedParams!=',' && *wantedParams != ')')
		{
			syslog("Wrong number of parameters while calling function.");
			AOEFREE(pName);
			delete params;
			return ScriptVariable();
		}

		// Associe le nom du parametre a sa valeur
		switch(ptype[pcount])
		{
			case 'i':
				params->setval(pName, va_arg(pv, int));
				break;

			case 's':
				params->setval(pName, va_arg(pv, char*));
				break;

			case 'o':
				params->setval(pName, va_arg(pv, ScriptableObject*));
				break;

			case 'p':
				Point* p = va_arg(pv, Point*);
				params->setval(pName, p->getX(), p->getY());
				break;
		}

		AOEFREE(pName);

		pcount++;
	}

	va_end(pv);
	// Lecture des parametres terminee

	return callFunctionWithParams(fvar, params);
}

// QC:L (leaking non verifie)
ScriptVariable Context::callFunction(char* fname, const char* ptype, ...)
{
	Context* params = new Context(engine());

	// Positionne le topContext
	params->setval("@tc", (char*)this, NULL, 0);

	// Recupere la fonction
	ScriptVariable* fvar = getVar(fname);
	if(fvar == NULL || (fvar->gettype() != VAR_FCT && fvar->gettype() != VAR_TXTFCT))
	{
		// La fonction n'existe pas
		delete params;
		return ScriptVariable();
	}

	// Lecture des parametres
	va_list pv; // Parameter Values
	va_start(pv, ptype);

	// Interprete les variables voulues par la fonction
	char* wantedParams = fvar->getparams();
	if(*wantedParams != '(')
	{
		delete params;
		return ScriptVariable();
	}

	ScriptedFunction::str_skipblanks(wantedParams);

	int pcount=0;
	while(*wantedParams && *wantedParams != ')')
	{
		// Saute le separateur
		wantedParams++;
		ScriptedFunction::str_skipblanks(wantedParams);

		// Lit le nom du parametre
		char* left = wantedParams;
		ScriptedFunction::str_skipvarname(wantedParams);
		char* pName = (char*)AOEMALLOC(wantedParams - left + 1);
		memcpy(pName, left, wantedParams - left);
		pName[wantedParams - left] = '\0';
		ScriptedFunction::str_skipblanks(wantedParams);

		// pName contient le nom du parametre

		if(*wantedParams!=',' && *wantedParams != ')')
		{
			syslog("Wrong number of parameters while calling function.");
			AOEFREE(pName);
			delete params;
			return ScriptVariable();
		}

		// Associe le nom du parametre a sa valeur
		switch(ptype[pcount])
		{
			case 'i':
				params->setval(pName, va_arg(pv, int));
				break;

			case 's':
				params->setval(pName, va_arg(pv, char*));
				break;

			case 'o':
				params->setval(pName, va_arg(pv, ScriptableObject*));
				break;

			case 'p':
				Point* p = va_arg(pv, Point*);
				params->setval(pName, p->getX(), p->getY());
				break;
		}

		AOEFREE(pName);

		pcount++;
	}

	va_end(pv);
	// Lecture des parametres terminee

	return callFunctionWithParams(fvar, params);
}

// QC:P
void Context::setMainThread(ScriptedFunction* newMainThread)
{
	mainThread = newMainThread;
}

// QC:G
void Context::load_script(VFS::File* sourceFile, char* code)
{
	setMainThread(new ScriptedFunction(engine(), sourceFile, code, this, NULL, true));
}


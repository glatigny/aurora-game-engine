#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "script_variable.h"
#include "scripted_function.h"
#include "context.h"
#include "vfs_file.h"


// QC:P
ScriptVariable::ScriptVariable() {
	type = VAR_NULL;
	value = 0;
}

// QC:G (operator= may not be perfect)
ScriptVariable::ScriptVariable(const ScriptVariable& v) {
	type = VAR_NULL;
	*this = v;
}

// QC:P
ScriptVariable::~ScriptVariable() {
	setType(VAR_NULL);
}

// L'operateur = detruit les pointeurs de la cible, contrairement a setval(ScriptVariable&)
// QC:G
ScriptVariable& ScriptVariable::operator=(const ScriptVariable& v)
{
	setType(v.gettype());

	char* str;

	//	params = NULL;
	//	data = NULL;
	//	value = 0;

	switch(type)
	{
		case VAR_NULL:
		break;

		case VAR_FCT:
		block = v.getblock();
		params = v.getparams();
		data = v.getdata();
		value = v.getint();
		break;

		case VAR_INT:
		value = v.getint();
		break;

		case VAR_PAIR:
		params = v.getparams();
		data = *(v.getdataptr());
		break;

		case VAR_RECT:
		if(data) {
			delete (Rect*)data;
		}
		data = (char*)(new Rect((Rect*)v.data));
		break;

		case VAR_STR:
		str = v.getstr();
		value = v.getint();

		if(data != NULL && params - data> value)
		{
			// Pas assez de place dans le buffer, on le supprime
			AOEFREE(data);
			data = NULL;
		}

		if(data == NULL)
		{
			int newBufSize = value + (value>128?256:64);
			data = (char*)AOEMALLOC(newBufSize);
			params = data + newBufSize; // params montre la fin de la chaine
		}

		memcpy(data, str, value);
		data[value] = '\0';
		assert((int)strlen(data) == value);

		break;

		case VAR_PACK:
		if(data) {
			AOEFREE(data);
		}
		if(params) {
			AOEFREE(params);
		}
		params = AOESTRDUP(v.getparams());
		value = v.getint();
		data = (char*)AOEMALLOC(value);
		memcpy(data, v.getdata(), value);

		break;

		// memcpy est generalement plus rapide que strcpy
		// le +1 copie le \0 en fin de chaine
		memcpy(data, str, value+1);
		break;

		case VAR_TXTFCT:
		value = v.getint();
		params = v.getparams();
		data = v.getstr();
		break;

		case VAR_XML:
		data = v.getdata();
		break;

		case VAR_OBJ:
		// objUnref done within setType().
		value = v.getint();
		params = v.getparams();
		data = v.getdata();
		trackPointer(data);
		break;

		case VAR_HTBL:
		data = v.getdata();
		break;

		case VAR_TBL:
		// Duplicate the table
		params = (char*)AOEMALLOC(sizeof(int));
		*(int*)params = 1;
		data = (char*) new ScriptTable(*(ScriptTable*)(v.getdata()));
		break;

		case VAR_RAW:
		params = v.getparams();
		data = v.getdata();
		value = v.getint();
		break;

		case VAR_PTR:
		// Ne devrait jamais arriver
		assert(false);
		break;
	}

	return *this;
}

// setType change le type d'une variable.
// Si la variable contient du contenu, il sera libere de la memoire.
// QC:P
void ScriptVariable::setType(varType t) {
	// Liberation des anciennes zones memoire
	switch(type){
	case VAR_STR:
		if(t != VAR_STR) {
			if(data) {
				AOEFREE(data);
				data = NULL;
			}
		}
		break;

	case VAR_PACK:
		if(data) {
			AOEFREE(data);
			data = NULL;
		}
		if(params) {
			AOEFREE(params);
			params = NULL;
		}
		value = 0;
		break;

	case VAR_OBJ:
		assert(data != NULL);
		untrackPointer(data);
		type = VAR_NULL;
		break;

	case VAR_HTBL:
		if(data != NULL) {
			type = VAR_NULL;
		}
		break;

	case VAR_RECT:
		if(data != NULL) {
			delete (Rect*) data;
		}
		break;

	case VAR_TBL:
		if(t != VAR_TBL) {
			freeTable(data, params); // Free with reference count.
		}

	case VAR_XML:
	case VAR_FCT:
	case VAR_RAW:
	case VAR_PTR:
	case VAR_TXTFCT:
	case VAR_INT:
	case VAR_PAIR:
	case VAR_NULL:
		break;
	}

	// Creation des nouvelles zones memoire
	switch(t){
	case VAR_STR:
		if(type != VAR_STR) {
			data = NULL;
		}
		break;

	case VAR_PACK:
		value = 0;

	case VAR_TBL:
		params = NULL;

	case VAR_HTBL:
	case VAR_RECT:
		data = NULL;

		break;

	case VAR_XML:
	case VAR_OBJ:
	case VAR_FCT:
	case VAR_RAW:
	case VAR_PTR:
	case VAR_TXTFCT:
	case VAR_INT:
	case VAR_PAIR:
	case VAR_NULL:
		break;
	}

	type = t;
}

// Micro-optimisation possible : reecriture en iteratif
// QC:S
ScriptVariable* ScriptVariable::getPointee() const {
	assert(data);

	if(((ScriptVariable*) data)->isPointer()) {
		return ((ScriptVariable*) data)->getPointee();
	}
	return (ScriptVariable*) data;
}

// QC:P
inline bool ScriptVariable::isPointer() const {
	return type == VAR_PTR;
}

/////////////////////////////////////////////////
// getXXX()

// QC:P
bool ScriptVariable::getbool() const {
	if(isPointer()) {
		return getPointee()->getbool();
	}

	switch(type){
	case VAR_NULL:
		return false;
	case VAR_INT:
		return value != 0;
	case VAR_PAIR:
		return data != 0 && params != 0;
	case VAR_STR:
		return value > 0;
	case VAR_PACK:
		return value != 0;
	case VAR_TBL:
		return data && ((ScriptTable*) data)->size();
	case VAR_RECT:
	case VAR_FCT:
	case VAR_HTBL:
	case VAR_XML:
	case VAR_OBJ:
	case VAR_RAW:
	case VAR_PTR:
	case VAR_TXTFCT:
		break;
	}

	return true;
}

// QC:P
#define GET(scripttype, var) \
ScriptVariable::get ## scripttype() const \
{ \
	if(isPointer()) return getPointee()->get ## scripttype(); \
	if(type == VAR_NULL) return 0; \
	return var; \
}
int* GET(intptr, (int*)&value)
char** GET(dataptr, (char**)&data)
char* GET(params, params)
char** GET(paramsptr, (char**)&params)
char* GET(class, params)
TiXmlNode* GET(xml, (TiXmlNode*)data);
#undef GET

// QC:P
Context* ScriptVariable::gethashtable() const {
	if(isPointer())
		return getPointee()->gethashtable();
	if(type != VAR_HTBL)
		return NULL;
	return (Context*) data;
}

// QC:P
ScriptableObject* ScriptVariable::getobj() const {
	if(isPointer())
		return getPointee()->getobj();
	if(type != VAR_OBJ)
		return NULL;
	return (ScriptableObject*) data;
}

// QC:P
int ScriptVariable::getint() const {
	if(isPointer())
		return getPointee()->getint();
	if(type == VAR_NULL) {
		return 0;
	} else if(type == VAR_PAIR) {
		// Quicker computation when only one component (frequent case)
		if(params == 0) {
			return (long) data;
		} else if(data == 0) {
			return (long) params;
		}

		return (int) sqrt((double) (((long) data) * ((long) data) + ((long) params) * ((long) params)));
	} else if(type == VAR_TBL) {
		return ((ScriptTable*) data)->size();
	}

	return value;
}

// QC:P
int ScriptVariable::getintPercent() const {
	if(isPointer())
		return getPointee()->getint();
	if(type == VAR_NULL) {
		return 100;
	} else if(type == VAR_PAIR) {
		// Quicker computation when only one component (frequent case)
		if(params == 0) {
			return (int)(long) data;
		} else if(data == 0) {
			return (int)(long) params;
		}

		return (int) sqrt((double) (((long) data) * ((long) data) + ((long) params) * ((long) params)));
	} else if(type == VAR_TBL) {
		return ((ScriptTable*) data)->size();
	}

	return value;
}

// QC:P
int ScriptVariable::getX() const {
	if(isPointer())
		return getPointee()->getX();
	if(type == VAR_RECT) {
		return ((Rect*) data)->getX();
	}

	if(type != VAR_PAIR) {
		if(type == VAR_INT) {
			return value;
		}

		return 0;
	}

	return (long) data;
}

// QC:P
int ScriptVariable::getY() const {
	if(isPointer())
		return getPointee()->getY();
	if(type == VAR_RECT) {
		return ((Rect*) data)->getY();
	}

	if(type != VAR_PAIR) {
		return 0;
	}

	return (long) params;
}

// QC:P
Point ScriptVariable::getPoint() const {
	if(isPointer())
		return getPointee()->getPoint();
	if(type != VAR_PAIR) {
		if(type == VAR_INT) {
			return Point(value, 0);
		}

		return Point();
	}

	return Point((long) data, (long) params);
}

// QC:P
Point ScriptVariable::getPointPercent() const {
	if(isPointer())
		return getPointee()->getPoint();
	if(type != VAR_PAIR) {
		if(type == VAR_INT) {
			return Point(value, 0);
		}

		return Point(100, 100);
	}

	return Point((long) data, (long) params);
}

// QC:?
Rect ScriptVariable::getRect() const {
	if(isPointer())
		return getPointee()->getRect();
	if(type != VAR_RECT) {
		if(type == VAR_PAIR) {
			return Rect((int) value, (int)(long) data, 0, 0);
		} else if(type == VAR_INT) {
			return Rect(value, 0, 0, 0);
		}

		return Rect();
	}
	return Rect((Rect*) data);
}

// QC:P
varType ScriptVariable::gettype() const {
	if(isPointer()) {
		return getPointee()->gettype();
	}

	return type;
}

// QC:A (discussion possible sur l'utilisation de snprintf. Les buffers statiques ne sont pas thread safe.)
char* ScriptVariable::getstr() const {
	if(isPointer()) {
		return getPointee()->getstr();
	}

	if(type == VAR_NULL) {
		return NULL;
	}

	if(type == VAR_INT) {
		static char number[32];
		snprintf(number, 32, "%d", value);
		number[31] ='\0';
		return number;
	}

	if(type == VAR_XML) {
		static TiXmlPrinter printer;
		printer = TiXmlPrinter();
		printer.SetStreamPrinting();
		TiXmlNode* node = (TiXmlNode*) data;
		node->Accept(&printer);
		return (char*)printer.CStr();
	}

	if(type == VAR_OBJ) {
		static char address[256];
		snprintf(address, 256, "(*%p:%s)", data, ((ScriptableObject*)data)->getClassName());
		address[255] ='\0';
		return address;
	}

	if(type == VAR_PAIR) {
		static char number[64];
		snprintf(number, 64, "(%ld:%ld)", (long) data, (long) params);
		number[63] ='\0';
		return number;
	}

	if(type == VAR_PACK) {
		static char info[4096]; // XXX : NOT thread safe
		snprintf(info, 4096, "(*%p => \"%s\")", data, params);
		info[4095] ='\0';
		return info;
	}

	if(type == VAR_TBL) {
		static char info[4096]; // XXX : NOT thread safe
		snprintf(info, 4096, "(tbl[%ld]:*%p)", (long)(data?(((ScriptTable*)data)->size()):0), data);
		info[4095] ='\0';
		return info;
	}

	if(type == VAR_FCT) {
		return params;
	}

	return data;
}

// QC:F (used in internal code)
char* ScriptVariable::getdata() const {
	if(isPointer()) {
		return getPointee()->getdata();
	}

	if(type == VAR_NULL) {
		return NULL;
	}

	if(type == VAR_INT) {
		static char number[128];
		sprintf(number, "%d", value);
		return number;
	}

	return data;
}

// QC:?
char* ScriptVariable::getblock() const {
	if(isPointer()) {
		return getPointee()->getblock();
	}

	if(type != VAR_FCT)
		return NULL;

	return block;
}

// QC:?
ScriptTable* ScriptVariable::gettable() const {
	if(isPointer()) {
		return getPointee()->gettable();
	}

	if(type != VAR_TBL) {
		return NULL;
	}

	return (ScriptTable*)data;
}

// QC:?
VFS::File* ScriptVariable::getfile(VFS::File* pathRoot) const {
	if(isPointer()) {
		return getPointee()->getfile();
	}

	if(type == VAR_OBJ) {
		return getobj()->source();
	}

	static VFS::File* f = NULL;
	if(f)
		delete f;
	f = VFS::openFile(getstr(), pathRoot);
	return f;
}

/////////////////////////////////////////////////
// setval()

// QC:P
void ScriptVariable::setval(void) {
	if(isPointer()) {
		getPointee()->setval();
		return;
	}

	setType(VAR_NULL);
}

// QC:P
void ScriptVariable::setval(const bool b) {
	if(isPointer()) {
		getPointee()->setval(b);
		return;
	}

	setType(VAR_INT);
	value = b ? 1 : 0;
}

// QC:P
void ScriptVariable::setval(const int i) {
	if(isPointer()) {
		getPointee()->setval(i);
		return;
	}

	setType(VAR_INT);
	value = i;
}

// QC:B
void ScriptVariable::setval(const float f) {
	if(isPointer()) {
		getPointee()->setval(f);
		return;
	}

	assert(false); // Not implemented
// TODO : implement VAR_FLOAT	setType(VAR_FLOAT);
}

// QC:B
void ScriptVariable::setval(const double d) {
	if(isPointer()) {
		getPointee()->setval(d);
		return;
	}

	assert(false); // Not implemented
// TODO : implement VAR_FLOAT	setType(VAR_FLOAT);
}

// QC:A (Encore pas checkée à 100%, des problèmes de gestion de buffers ont été trouvés)
void ScriptVariable::setval(const char* str) {
	if(isPointer()) {
		getPointee()->setval(str);
		return;
	}

	setType(VAR_STR);
	value = strlen(str);

	if(data != NULL && data + value >= params) {
		// Pas assez de place dans le buffer, on le supprime
		AOEFREE(data);
		data = NULL;
	}

	if(data == NULL) {
		int newBufSize = value + (value > 48 ? 255 : 32);
		data = (char*) AOEMALLOC(newBufSize + 1);
		params = data + newBufSize; // params montre la fin du buffer
	}

	// memmove est generalement plus rapide que strcpy
	// le +1 copie le \0 en fin de chaine
	memmove(data, str, value + 1);
}

// QC:?
void ScriptVariable::setval(const char* format, const char* content) {
	if(isPointer()) {
		getPointee()->setval(format, content);
		return;
	}

	setType(VAR_PACK);

	if(format && content && *format) {
		params = AOESTRDUP(format);
		data = packedStructureDup(format, content);
		value = getPackedStructureLength(params, data);
	}
}

// QC:P
void ScriptVariable::setval(const int x, const int y) {
	if(isPointer()) {
		getPointee()->setval(x, y);
		return;
	}

	setType(VAR_PAIR);

#ifdef FLOAT_COORD
	data = (char*)(long) x;
	params = (char*)(long) y;
#else
	data = (char*) x;
	params = (char*) y;
#endif
}

// QC:A (le param devrait etre une reference const. data et params ne suffisent pas pour stocker un double)
void ScriptVariable::setval(Point p) {
	if(isPointer()) {
		getPointee()->setval(p);
		return;
	}

	setType(VAR_PAIR);

#ifdef FLOAT_COORD
	data = (char*)(long) p.getX();
	params = (char*)(long) p.getY();
#else
	data = (char*) p.getX();
	params = (char*) p.getY();
#endif
}

// QC:S (le param devrait etre une reference const. Fonctionnelement parfaite)
void ScriptVariable::setval(Rect p) {
	if(isPointer()) {
		getPointee()->setval(p);
		return;
	}

	setType(VAR_RECT);

	data = (char*) new Rect(p);
}

// QC:S (le param devrait etre une reference const. Fonctionnelement parfaite)
void ScriptVariable::setval(Point p, Point s) {
	if(isPointer()) {
		getPointee()->setval(p, s);
		return;
	}

	setType(VAR_RECT);

	data = (char*) new Rect(p, s);
}

// QC:?
void ScriptVariable::setval(int x, int y, int w, int h) {
	if(isPointer()) {
		getPointee()->setval(x, y, w, h);
		return;
	}

	setType(VAR_RECT);

	data = (char*) new Rect(x, y, w, h);
}

// QC:F (cette fonction est un hack, elle ne peut etre parfaite. A considerer comme fonctionnelement parfaite)
void ScriptVariable::setval(char* d, char* p, int v) {
	if(isPointer()) {
		getPointee()->setval(d, p, v);
		return;
	}

	setType(VAR_RAW);
	data = d;
	params = p;
	value = v;
}

// QC:P
void ScriptVariable::setval(Context* c) {
	if(isPointer()) {
		getPointee()->setval(c);
		return;
	}

	setType(VAR_HTBL);
	data = (char*) c;
}

// QC:?
void ScriptVariable::setval(ScriptTable* c) {
	if(isPointer()) {
		getPointee()->setval(c);
		return;
	}

	setType(VAR_TBL);

	if(data) {
		freeTable(data, params);
	}

	data = (char*)c;
	params = (char*) AOEMALLOC(sizeof(int));
	*(int*) params = 1;
}

// QC:?
void ScriptVariable::setval(State* attachedState, VFS::File* f) {
	VfsFile* vf = new VfsFile(attachedState, f);
	setval(vf);
}

// QC:P
void ScriptVariable::settxtfct(char* code, char* fparams, const int flags) {
	if(isPointer()) {
		getPointee()->settxtfct(code, fparams, flags);
		return;
	}

	setType(VAR_TXTFCT);

	data = code;
	params = fparams;
	value = flags;
}

// setval est differente de operator= car si la variable est un pointeur, c'est
// la variable pointee qui est modifiee.
// QC:P
void ScriptVariable::setval(ScriptVariable& v) {
	if(isPointer()) {
		getPointee()->setval(v);
		return;
	}

	switch(v.type){
	case VAR_NULL:
		setType(VAR_NULL);
		break;

	case VAR_FCT:
		setfct(v.data, (const char*) v.params, v.value, v.block);
		break;

	case VAR_INT:
		setType(VAR_INT);
		value = v.value;
		break;

	case VAR_PAIR:
		setType(VAR_PAIR);
		params = v.params;
		data = v.data;
		break;

	case VAR_RECT:
		if(data) {
			delete (Rect*) data;
		}
		data = (char*) (new Rect((Rect*) v.data));
		break;

	case VAR_STR:
		setType(VAR_STR);
		setval(v.data);
		break;

	case VAR_PACK:
		setType(VAR_PACK);
		if(data) {
			AOEFREE(data);
		}
		if(params) {
			AOEFREE(params);
		}
		params = AOESTRDUP(v.params);
		value = v.value;
		data = (char*) AOEMALLOC(value);
		memcpy(data, v.data, value);
		break;

	case VAR_TXTFCT:
		setType(VAR_TXTFCT);
		settxtfct(v.data, v.params, v.value);
		break;

	case VAR_HTBL:
		setType(VAR_HTBL);
		data = v.data;
		break;

	case VAR_TBL:
		setType(VAR_TBL);
		data = v.data;
		params = v.params;
		(*(int*)params)++;
		break;

	case VAR_XML:
		setType(VAR_XML);
		data = v.data;
		break;

	case VAR_OBJ:
		setType(VAR_OBJ);
		params = v.params;
		data = v.data;
		trackPointer(data);
		break;

	case VAR_RAW:
		setType(VAR_RAW);
		params = v.params;
		data = v.data;
		value = v.value;
		break;

	case VAR_PTR:
		// Dereference le pointeur
		setval(*(v.getPointee()));
		break;
	}
}

// QC:?
void ScriptVariable::setval(TiXmlNode* xml) {
	if(isPointer()) {
		getPointee()->setval(xml);
		return;
	}

	if(xml) {
		setType(VAR_XML);
		data = (char*)xml;
	} else {
		setType(VAR_NULL);
	}
}

// QC:L
void ScriptVariable::setval(ScriptableObject* ptr) {
	if(isPointer()) {
		getPointee()->setval(ptr);
		return;
	}

	if(type == VAR_OBJ && data == (char*) ptr) {
		return;
	}

	if(ptr) {
		setType(VAR_OBJ);

		data = (char*) ptr;
		params = (char*) ptr->getClassName();

		trackPointer(data);
	} else {
		setType(VAR_NULL);
	}
}

// QC:P
void ScriptVariable::setfct(char* callback_ptr, const char* lparams, const int flags, char* block_callback_ptr) {
	assert(callback_ptr);
	assert(lparams);

	if(isPointer()) {
		getPointee()->setfct(callback_ptr, lparams, flags, block_callback_ptr);
		return;
	}

	setType(VAR_FCT);

	value = flags;
	data = callback_ptr;
	params = (char*) lparams;
	block = block_callback_ptr;
}

// QC:P
void ScriptVariable::setptr(const ScriptVariable& v) {
	if(isPointer()) {
		getPointee()->setptr(v);
		return;
	}

	setType(VAR_PTR);
	data = (char*) (&v);
}

// QC:G (l'allocation memoire devrait etre partagee avec setval(char*))
void ScriptVariable::concat(const char* str) {
	assert(type == VAR_NULL
	|| type == VAR_INT
	|| type == VAR_STR
	|| type == VAR_PAIR);

	if(isPointer()) {
		getPointee()->concat(str);
		return;
	}

	// Si la variable est de type null, on la convertit en string vide
	if(type == VAR_NULL) {
		setval("");
	}

	// Si la variable est de type int, on la convertit en string
	if(type == VAR_INT || type == VAR_PAIR) {
		setval(getstr());
	}

	int slen = strlen(str);
	if(data + value + slen >= params) {
		// Reallocation d'une nouvelle zone plus grande
		int newBufSize = ((value + slen) / 256 + 2) * 256;
		char* newData = (char*) AOEMALLOC(newBufSize);

		// Copie de la chaine actuelle
		memcpy(newData, data, value);
		newData[value] = '\0';

		// Remplace le buffer actuel par le nouveau
		AOEFREE(data);
		data = newData;
		params = data + newBufSize;

		assert(newBufSize >= value + slen + 1);
	}

	// La zone est assez grande pour contenir les 2 chaines.
	// On copie la deuxieme chaine apres la premiere
	memcpy(data + value, str, slen + 1);
	value += slen;

	assert(data + value <= params);
}

// QC:A
void ScriptVariable::unlinkPtr() {
	if(isPointer()) {
		ScriptVariable v(*getPointee());
		*this = v;
	}
}

// QC:?
void ScriptVariable::freeTable(char* &data, char* &params) {
	if(!(*(int*)params)--) {
		AOEFREE(params);
		delete (ScriptTable*)data;
		params = data = NULL;
	}
}

// QC:P
void ScriptVariable::reset() {
	setType(VAR_NULL);
}

// QC:G
void ScriptVariable::callNativeFunction(Context* params, Context* tc) {
	if(isPointer()) {
		getPointee()->callNativeFunction(params, tc);
		return;
	}

	assert(type == VAR_FCT || type == VAR_TXTFCT);

	params->registerScriptVariable("@retval");
	if(type == VAR_FCT) {
		ScriptVariable* retVal = params->getVar("@retval");

		if(value & FCTFLAG_STATIC) {
			// Appel direct de la fonction statique
			((void(*)(ScriptVariable&, Context&)) data)(*retVal, *params);
		} else {
			ScriptableObject* scriptThis = (ScriptableObject*) (((*params)["this"])->getobj());

			if(scriptThis != NULL) {
				// Appel indirect de la fonction membre
				((void(*)(ScriptableObject*, ScriptVariable&, Context&)) data)(scriptThis, *retVal, *params);
			}
		}
	} else // type == VAR_TXTFCT
	{
		assert(!(value & FCTFLAG_BLOCKING));
		assert(!(value & FCTFLAG_CLASS));
		ScriptedFunction* tmpThread = new ScriptedFunction(params->engine(), NULL, getstr(), params, tc, false, true);
#ifndef NDEBUG
		bool blocked = 
#endif
		tmpThread->run();
		assert(!blocked);
	}
}

bool ScriptVariable::queryFunctionBlock(Context* params) {
	if(isPointer()) {
		return getPointee()->queryFunctionBlock(params);
	}

	assert(type == VAR_FCT);
	assert(value & FCTFLAG_BLOCKING);

	if(value & FCTFLAG_STATIC) {
		return ((bool(*)(Context&)) block)(*params);
	} else {
		ScriptableObject* scriptThis = (ScriptableObject*) (((*params)["this"])->getobj());
		return ((bool(*)(ScriptableObject*, Context&)) block)(scriptThis, *params);
	}
}

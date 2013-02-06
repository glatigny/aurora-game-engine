#ifndef _Variable__
#define _Variable__
class ScriptVariable;
class ScriptableObject;
class ScriptedFunction;
class Context;
class State;

// Flags des fonctions
#define FCTFLAG_NORMAL 0
#define FCTFLAG_STATIC 1
#define FCTFLAG_BLOCKING 2
#define FCTFLAG_CLASS 4

#include <vector>
#include "tinyxml.h"

typedef std::vector<ScriptVariable> ScriptTable;

#include "interface.h"
#include "translatable.h"
#include "scriptable_object.h"
#include "rect.h"

/** Types that a scriptvariable can have. */
typedef enum _varType {
		VAR_NULL,  // ScriptVariable sans valeur
		VAR_RAW,   // Valeur brute (2 pointeurs et 1 int)
		VAR_FCT,   // Fonction native
		VAR_INT,   // Entier signe
//		VAR_FLOAT, // Virgule flottante (TODO: implement float in script engine)
		VAR_PAIR,  // Paire
		VAR_RECT,  // Rectangle zone
		VAR_STR,   // Chaine de caracteres
		VAR_PACK,  // Structure packee
		VAR_TXTFCT,// Fonction scriptee
		VAR_HTBL,  // Table de hashage
		VAR_TBL,   // Table
		VAR_XML,   // XML node
		VAR_OBJ,   // Objet scriptable
		VAR_PTR,   // Pointeur vers une ScriptVariable
} varType;

/** Stores a SSS variable.
Variables are universal placeholders for any kind of thing in SSS.
They store values, functions, objects and pointers.
@see varType
*/
class ScriptVariable : public Interface
{
friend class ScriptedFunction;
private:
	varType type;
	int value; // Entier ou taille de la chaine ou flags de la fonction
	char* data; // Chaine ou texte de la fonction scriptee ou pointeur vers l'objet natif ou pointeur vers la scriptvariable ou pointeur vers l'objet thread
	char* params; // Parametres de la fonction ou classe de l'objet
	char* block; // Blocking function

	void setType(varType t); // Change la scriptvariable de type
	ScriptVariable* getPointee() const; // Retourne la valeur pointee

	void freeTable(char* &data, char* &params);
public:
	ScriptVariable();
	ScriptVariable(const ScriptVariable&); // Copie de variable

	~ScriptVariable();

	ScriptVariable& operator=(const ScriptVariable& v);

	/** Tells if a scriptvariable is a pointer.
	This function is the only way to tell if a scriptvariable is a pointer or not.
	@return Returns true if theScriptVariable is a pointer (VAR_PTR), false otherwise.
	@see gettype()
	*/
	bool isPointer() const;

	/** Returns the type of this variable.
	This function is pointer-transparent : it returns the type of the pointed value.
	@return Returns the type of the variable.
	@see isPointer()
	*/
	varType gettype() const; // Si la scriptvariable est un pointeur, renvoie le type de la scriptvariable pointee

	// Donnees
	bool getbool() const;
	int getint() const;
	int getintPercent() const;
	int* getintptr() const;
	coord getcoord() const {return (coord)getint();}
	dur getdur() const {return (dur)getint();}
	char* getstr() const;
	char* getdata() const;
	char* getblock() const;
	char** getdataptr() const;
	char* getparams() const;
	char** getparamsptr() const;
	Context* gethashtable() const;
	ScriptTable* gettable() const;
	ScriptableObject* getobj() const;
	TiXmlNode* getxml() const;
	VFS::File* getfile(VFS::File* pathRoot = NULL) const;
	Point getPoint() const;
	Point getPointPercent() const;
	Rect getRect() const;
	int getX() const;
	int getY() const;
	char* getclass() const;

	// Setval
	void setval(void);
	void setval(const bool);
	void setval(const int);
	void setval(const int, const int);
	void setval(const float);
	void setval(const double);
	void setval(Point p); // Paire
	void setval(Rect r); // Rectangle
	void setval(Point p, Point s); // Rectangle
	void setval(int, int, int, int); // Rectangle
	void setval(const char*); // Chaine
	void setval(Context* c); // Hash Table
	void setval(ScriptTable* t); // Table
	void setval(ScriptableObject* obj); // Objet scriptable
	void setval(TiXmlNode* xml); // XML tag
	void setval(State* attachedState, VFS::File* f); // File
	void setval(const char*, const char*); // Structure packee
	void settxtfct(char* code, char* params, const int flags); // Fonction scriptee
	void setval(char* dataptr, char* paramsptr, int value); // Pointeurs bruts
	void setval(ScriptVariable& v); // Recopie
	void setptr(const ScriptVariable& v); // Pointeur vers une ScriptVariable
	void setfct(char* callback_ptr, const char* params, const int flags, char* block_callback_ptr); // Fonction native
	void concat(const char* str); // Concatenation de strings

	/** Transforms pointer to values.
	If the variable is a pointer, it takes the value of the pointed variable.
	If the variable is not a pointer, nothing happens.
	@see isPointer()
	@see reset()
	*/
	void unlinkPtr();

	/** Resets variable, destroying pointers.
	The variable will be reset (set to VAR_NULL type).
	If the variable is a pointer, only the pointer is destroyed, leaving
	the pointee alone.
	@see unlinkPtr()
	*/
	void reset();

	/** Calls a non-blocking function.
	*/
	void callNativeFunction(Context* params, Context* tc = NULL);

	/** Tells if a function is blocking.
	 */
	bool queryFunctionBlock(Context* params);
};

#endif


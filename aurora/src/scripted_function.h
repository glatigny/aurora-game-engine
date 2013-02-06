#ifndef SCRIPTED_FUNCTION_H
#define SCRIPTED_FUNCTION_H
class ScriptedFunction;
class Context;
class ScriptVariable;

#include "abstract.h"
#include "scriptable_object.h"
#include "script_variable.h"
#include "blocking.h"

#include <list>

typedef int operatorType;

/** SSS interpreter.
 This class interprets code in a given context.
 */
class ScriptedFunction : public Abstract, public ScriptableObject, public Blocking {
	friend class Context;
private:
	ScriptedFunction* sub; // Subblock
	ScriptedFunction* superThread; // Thread that executes this one
	Context* tc; // Execution context
	Context* ec; // Execution context
	bool oneInstr; // Only one instruction to execute
	bool oneInstrDone; // One instruction has been executed
	bool running; // The function is in a running state
	bool retint; // Return from interrupt
	char* scriptStart; // Used for serialization to compute real ptr position
	Context* scriptHolder; // Context holding currently running script
	char* scriptHolderVariable; // Variable name of the currently running script
	char* startPtr; // Execution entry point
	char* ptr; // Actual execution pointer
	const char* instr; // Instruction currently being executed
	bool ownEC; // ec should be deleted after use.

	std::list<ScriptedFunction*> subThreads; // List of threads managed by this ScriptedFunction

	int loopCnt;
	int pauseCnt;

	union {
		struct {
			Context* fctParams;
			ScriptVariable* fctVar;
			char* fctPtr;
			Context* htbl;
			Context* oldsuper;
		} instr_fct;
		struct {
			bool exprResult;
			char* codeblock;
		} instr_if;
		char* instr_for[4]; // Data pointers used to remember execution status
		struct {
			char* condition;
			char* codeblock;
			bool isUntil;
		} instr_while;
		struct {
			Context* currentContext;
		} instr_dot;
		struct {
			char* codeblock;
			unsigned int current_position;
			char* value_variable;
			char* key_variable;
			ScriptVariable* table;
		} instr_foreach;
	} instr_data;
	ScriptVariable lastObject;

	// Controle d'execution
	bool subRun(bool runCode = true);
	bool subRun(const char* start, bool runCode = true);
	bool subRun(const char* start, Context* superContext, bool runCode = true);
	bool subFunctionRun(const char* start, bool runCode, Context* params, bool chain = false);
	bool chain(const bool runCode = true);
	void restart();
	void finish();
	void addThread(ScriptedFunction* newThread); // Declare a new active thread. It will remove itself when finished.
	void removeThread(const ScriptedFunction* thread);
	bool threadsRunning();
	void setSuperThread(ScriptedFunction* superThread);

	// Instructions
	void setInstr(const char* newInstr);
	bool instrIf(const bool runCode);
	bool instrFor(const bool runCode);
	bool instrDo(const bool runCode);
	bool instrWhile(const bool runCode, bool isUntil);
	bool instrUsing(const bool runCode);
	bool instrForeach(const bool runCode);

	// Parseur de declarations
	void declare_var(const bool runCode);
	void declareFunction(const bool runCode, const bool isClass = false);
	void declareScriptThread(const bool runCode);

	// Parseur d'expression
	bool parseExpr(bool runCode, bool returnExpressionResult = false);
	bool parseExpr(char* start, bool runCode, bool returnExpressionResult = false);
	bool subParse(bool runCode, ScriptVariable& retVal, const operatorType opPriority);
	bool subParse(char* ptr, bool runCode, ScriptVariable& retVal, const operatorType opPriority);
	operatorType findOp(ScriptVariable &retVal, char* &string, operatorType opPriority);
	operatorType op2OpType(char* &srcop);
	void unifyTypes(ScriptVariable& l, ScriptVariable& r);
	void parsePack(bool runCode, ScriptVariable& retVal);
	void parseUnpack(bool runCode);

	// Parseur de chaines
	bool instrCmp(char* &ptr, const char* instr);
	char* str_tokens(char* &ptr, const char* sepList, bool eliminateSpecialChars);
	char* str_dupvarname(char* &ptr, bool memberFunction = false);
	char* find_code_end(char* start);
	static char* str_skipblanks(char* &ptr);
	static char* str_skipvarname(char* &ptr);

	// Gestion des erreurs
	void syntaxError(char* position, const char* message);

public:
#define HEADER scripted_function
#define HELPER scriptable_object_h
#include "include_helper.h"
	ScriptedFunction() : sub(NULL), ec(NULL), instr(NULL), ownEC(false) {}
	ScriptedFunction(AuroraEngine* parentEngine, VFS::File* source, const char* start, Context* superContext, Context* topContext = NULL, bool chain = false, bool oneInstr = false, ScriptedFunction* superThread = NULL);
	~ScriptedFunction();

	// Controle d'execution
	bool run();
	bool framestep();
	bool isRunning();
	bool isFinished() {
		return !isRunning();
	}

	// Gestion de l'ordonnanceur de threads
	void go();
	void stop();
	void kill();
	void pause(int times);
	void loop(int times);
	bool waitForTermination();
	bool yield();
	bool hasSubThread(const ScriptedFunction* subThread);
};

#endif


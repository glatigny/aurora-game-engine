#include "scripted_function.h"
#include "script_debugger.h"
#include "aurora_engine.h"
#include "context.h"
#define HEADER scripted_function
#define HELPER scriptable_object_cpp
#include "include_helper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#define LOAD_BLOCK_SIZE 4096

#ifdef SCRIPTLOG
#define scriptlog(msg) if(runCode) std::cerr << msg << " ";
#else
#define scriptlog(msg)
#endif

#define INSTR_CMP(myinstr) ((retint && instr && runCode)?(strcmp(myinstr, instr)==0):instrCmp(ptr, myinstr))

/** List of all the available operators, sorted by priority.*/
static const char* opList[] = {
"  ", // Operateur nul
"} ", "; ", ", ", ") ", "] ", "# ", "%=", "/=", "*=", "-=", "+=", "~=", "^=", "= ", "||", "&&", "!=", "==", "< ", "<=", ">=", "> ",
"~ ", "^ ", "!-", "+ ", "% ", "/ ", "* ", ": ", "! ", "- ", "--", "++", "$ ", "[ ", "@ ", "( ", ". ", "\" " };

/* Order of operators.
 This is also used for priority and must match opList[]
 */
#define OP_NUL 0
#define OP_BRC_CLOSE 1
#define OP_SEMICOLON 2
#define OP_COMMA 3
#define OP_PAR_CLOSE 4
#define OP_SBR_CLOSE 5
#define OP_RUN 6
#define OP_MOD_SET 7
#define OP_DIV_SET 8
#define OP_MUL_SET 9
#define OP_SUB_SET 10
#define OP_ADD_SET 11
#define OP_MIN_SET 12
#define OP_MAX_SET 13
#define OP_SET 14
#define OP_OR 15
#define OP_AND 16
#define OP_NOT_EQUAL 17
#define OP_EQUALS 18
#define OP_LT 19
#define OP_LT_EQ 20
#define OP_GT_EQ 21
#define OP_GT 22
#define OP_MIN 23
#define OP_MAX 24
#define OP_SUB 25
#define OP_ADD 26
#define OP_MOD 27
#define OP_DIV 28
#define OP_MUL 29
#define OP_PAIR 30
#define OP_NOT 31
#define OP_NEG 32
#define OP_DEC 33
#define OP_INC 34
#define OP_EVAL 35
#define OP_SBR 36
#define OP_FILE 37
#define OP_PAR 38
#define OP_DOT 39
#define OP_STRING 40
// Nombre d'operateurs :
#define OP_COUNT 41

// QC:G
ScriptedFunction::ScriptedFunction(AuroraEngine* parentEngine, VFS::File* sourceFile, const char* start, Context* superContext, Context* topContext, bool chain, bool newOneInstr, ScriptedFunction* newSuperThread) : Abstract(parentEngine), ScriptableObject(sourceFile)
{
	oneInstrDone = false;
	startPtr = (char*) start; // FIXME:Ugly cast
	instr = NULL;
	oneInstr = newOneInstr;
	pauseCnt = 0;
	loopCnt = 0;
	running = false;

	if(chain) {
		ownEC = false;
		ec = superContext;
	} else {
		ownEC = true;
		ec = new Context(engine());
		ec->setsuper(superContext);
	}

	assert(ec);

	tc = topContext ? topContext : superContext;
	assert(tc);

	superThread = NULL;
	setSuperThread(newSuperThread);

	ec->registerScriptVariable("thisthread")->setval(this);

	restart();

	sub = NULL;
}

ScriptedFunction::~ScriptedFunction() {
	delete sub;

	setInstr(NULL);

	if(!subThreads.empty())
		for(std::list<ScriptedFunction*>::iterator i = subThreads.begin(); i != subThreads.end(); i++) {
			delete *i;
		}

	if(ownEC)
		delete ec;

}

void ScriptedFunction::setSuperThread(ScriptedFunction* newSuperThread) {
	superThread = newSuperThread;
}

// QC:P
bool ScriptedFunction::subRun(const char* start, bool runCode) {
	return subRun(start, ec, runCode);
}

// QC:P
bool ScriptedFunction::subRun(bool runCode) {
	return subRun(ptr, ec, runCode);
}

// QC:G
bool ScriptedFunction::subRun(const char* start, Context* superContext, bool runCode) {
	if(runCode) {
		assert(!sub);

		sub = new ScriptedFunction(engine(), source(), start, superContext, tc, false, true);
		if(sub->run()) {
			return true;
		}

		ptr = sub->ptr;
		delete sub;
		sub = NULL;
	} else {
		bool oneInstrStore = oneInstr;
		bool oneInstrDoneStore = oneInstrDone;
		oneInstr = true;

		chain(false);

		oneInstr = oneInstrStore;
		oneInstrDone = oneInstrDoneStore;
	}

	return false;
}

// QC:G
bool ScriptedFunction::subFunctionRun(const char* start, bool runCode, Context* params, bool chainExecution) {
	if(runCode) {
		assert(!sub);

		params->registerScriptVariable("@retval"); // XXX : Necessary ?
		sub = new ScriptedFunction(engine(), source(), start, params, tc, chainExecution, true);
		if(sub->run()) {
			return true;
		}

		delete sub;
		sub = NULL;
	} else {
		chain(false);
	}

	return false;
}

// QC:P
void ScriptedFunction::loop(int times) {
	loopCnt = times;
}

// QC:P
void ScriptedFunction::pause(int times) {
	pauseCnt = times;
}

// QC:P
bool ScriptedFunction::framestep() {
	// Implement pause
	if(pauseCnt) {
		if(pauseCnt > 0)
			--pauseCnt;
		return true;
	}

	// Run the code to be executed in this thread
	bool interrupted = run();

	// Implement loop
	if(!interrupted) {
		if(loopCnt) {
			restart(); // Restart execution from entry point
			if(loopCnt > 0) {
				--loopCnt;
			}
			return true;
		}
	}

	return interrupted;
}

// QC:L
void ScriptedFunction::restart() {
	if(!isRunning()) {
		ptr = startPtr;
		running = true;
		if(superThread) {
			superThread->addThread(this);
		}
		retint = false;
	}
	xassert(isRunning(), "Could not start script thread.");
}

// QC:A
bool ScriptedFunction::run() {
	// Make subThreads run
	std::list<ScriptedFunction*> finishedThreads;

	if(!subThreads.empty())
		for(std::list<ScriptedFunction*>::iterator i = subThreads.begin(); i != subThreads.end(); i++) {

			bool subThreadInterrupted = (*i)->framestep();

			if(!subThreadInterrupted) {
				// This subThread finished.
				finishedThreads.push_front(*i);
			}
		}

	// Delete finished threads.
	if(!finishedThreads.empty())
		for(std::list<ScriptedFunction*>::iterator i = finishedThreads.begin(); i != finishedThreads.end(); i++) {
			if(!(*i)->isRunning()) {
				subThreads.remove(*i);
				delete *i;
			}
		}

	if(running) {
		if(sub) {
			// Run the subblock
			if(sub->framestep()) {
				return true;
			}

			ptr = sub->ptr;
			delete sub;
			sub = NULL;
		}

		if(chain(true)) {
			retint = true;
			return true;
		}

		// function execution terminated correctly
		running = false;
	}

	// If there are still threads running, mark the thread as interrupted
	return threadsRunning();
}

// QC:P
bool ScriptedFunction::isRunning() {
	return running || threadsRunning();
}

// QC:P
bool ScriptedFunction::threadsRunning() {
	if(running)
		return true;

	if(!subThreads.empty())
		for(std::list<ScriptedFunction*>::iterator i = subThreads.begin(); i != subThreads.end(); i++) {
			if((*i)->threadsRunning()) {
				return true;
			}
		}

	return false;
}

// QC:?
bool ScriptedFunction::hasSubThread(const ScriptedFunction* subThread) {
	if(!subThreads.empty())
		for(std::list<ScriptedFunction*>::iterator i = subThreads.begin(); i != subThreads.end(); i++) {
			if(*i == subThread) {
				return true;
			}
		}

	return false;
}

// QC:P
void ScriptedFunction::addThread(ScriptedFunction* newThread) {
	assert(newThread);
	if(!hasSubThread(newThread)) {
		subThreads.push_back(newThread);
	}
}

// QC:?
void ScriptedFunction::removeThread(const ScriptedFunction* thread) {
	assert(hasSubThread(thread));
	if(!subThreads.empty())
		for(std::list<ScriptedFunction*>::iterator i = subThreads.begin(); i != subThreads.end(); i++) {
			if(*i == thread) {
				subThreads.erase(i);
				return;
			}
		}
}

// QC:P
inline void ScriptedFunction::go() {
	pause(0);
	if(!isRunning()) {
		restart();
	}
}

// QC:P
inline void ScriptedFunction::stop() {
	pause(-1);
	loop(0);
}

// QC:U
void ScriptedFunction::kill() {
	running = false;

	// Kill sub threads
	if(!subThreads.empty())
		for(std::list<ScriptedFunction*>::iterator i = subThreads.begin(); i != subThreads.end(); i++) {
			(*i)->kill();
			delete *i;
		}
	subThreads.clear();

	finish();
}

void ScriptedFunction::finish() {
	// Kill sub
	if(sub) {
		sub->kill();
		delete sub;
		sub = NULL;
	}

	// Stop current instruction
	setInstr(NULL);

	// Remove the thread from superThread
	if(superThread) {
		superThread->removeThread(this);
	}
}

// QC:P
bool ScriptedFunction::waitForTermination() {
	return isRunning();
}

// QC:P
bool ScriptedFunction::yield() {
	return (pauseCnt != 0);
}

// QC:G
bool ScriptedFunction::chain(const bool runCode) {
	oneInstrDone = false;

	if(!retint || !runCode) {
		// On est en debut d'appel de code

		// Recherche du marqueur de fin de bloc
		str_skipblanks(ptr);
		if(*ptr == '{') {
			oneInstr = false; // Si c'est un bloc, on execute forcement plusieurs instructions
			ptr++; // On saute l'accolade de debut
			scriptlog("{");
		}

		if(runCode) {
			setInstr(NULL);
		}
	} else {
		// On est en retour d'interruption

		// Si on est en appel de fonction bloquante, meme pas la peine d'aller plus loin :
		if(instr) {
			if(strcmp(instr, "fct") == 0) {
				scriptlog("Resuming blocking native function call");

				bool blocks = instr_data.instr_fct.fctVar->queryFunctionBlock(instr_data.instr_fct.fctParams);

				// Tant que la fonction renvoie "true", on bloque l'interpreteur.
				if(blocks) {
					return true;
				}

				ptr = instr_data.instr_fct.fctPtr;
				setInstr(NULL);
				retint = false;

				//XXX return false;
			} else if(strcmp(instr, "dot") == 0) {
				setInstr(NULL);
			} else if(strcmp(instr, "txtfct") == 0) {
				ptr = instr_data.instr_fct.fctPtr;

				ScriptVariable* thisVar = NULL;
				if(!(instr_data.instr_fct.fctVar->getint() & FCTFLAG_STATIC)) {
					thisVar = ec->getVar("this");
				}
				if(thisVar && thisVar->gettype() == VAR_HTBL) {
					Context* htbl = thisVar->gethashtable();
					if(htbl->getsuper()) {
						htbl->setsuper(instr_data.instr_fct.fctParams);
					}
				}
				setInstr(NULL);
			}
			lastObject.reset();
		}
	}

	if(runCode)
		lastObject.reset();

	///////////
	// Boucle principale de l'interpreteur

	while((retint && instr && runCode) || (*str_skipblanks(ptr) && !(*ptr == '}' || (oneInstr && oneInstrDone)))) {
#ifdef SCRIPT_DEBUGGER_SUPPORT
		if(runCode)
		{
			if(ScriptDebugger::scriptDebugStep(ptr, ec))
			return true;
		}
#endif
		bool isUntil = false;

		// Teste si l'instruction est un bloc de code
		if(instr == NULL && *ptr == '{') {
			if(subRun(runCode))
				return true;
		}
		// Teste si l'instruction est de type imperatif (if, for, ...)
		else if(INSTR_CMP("if")) {
			scriptlog("if");
			if(instrIf(runCode))
				return true;
		} else if(INSTR_CMP("else")) {
			syntaxError(ptr, "Misplaced else.");
		} else if(INSTR_CMP("for")) {
			scriptlog("for");
			if(instrFor(runCode))
				return true;
		} else if(INSTR_CMP("do")) {
			scriptlog("do");
			if(instrDo(runCode))
				return true;
		} else if(INSTR_CMP("while") || ((isUntil = true) && INSTR_CMP("until"))) // le "isUntil=true" est volontaire
		{
			scriptlog((isUntil?"until":"while"));
			if(instrWhile(runCode, isUntil))
				return true;
		} else if(INSTR_CMP("foreach")) {
			scriptlog("foreach");
			if(instrForeach(runCode))
				return true;
		} else if(instrCmp(ptr, "using")) {
			scriptlog("using");
			if(instrUsing(runCode))
				return true;
		} else if(instrCmp(ptr, "unpack")) {
			scriptlog("unpack");
			parseUnpack(runCode);
		} else if(instrCmp(ptr, "return")) {
			scriptlog("return");

			if(runCode) {
				// Calcule la valeur de retour
				ScriptVariable* returnValue = ec->getVar("@retval");
				assert(returnValue);
				if(subParse(runCode, *returnValue, OP_NUL)) {
					syntaxError(ptr, "Blocking functions have no return value.");
				}

				returnValue->unlinkPtr();

				// Quitte l'execution en cours
				retint = false;
				return false;
			}
		} else if(instrCmp(ptr, "function")) {
			scriptlog("function");
			declareFunction(runCode);
		} else if(instrCmp(ptr, "class")) {
			scriptlog("class");
			declareFunction(runCode, true);
		} else if(instrCmp(ptr, "thread")) {
			scriptlog("thread");
			declareScriptThread(runCode);
		} else if(instrCmp(ptr, "var")) {
			scriptlog("var");
			declare_var(runCode);
		} else {
			// Ceci est une expression
			// On la parse
			if(parseExpr(runCode))
				return true;
		}

		scriptlog("\n");
		oneInstrDone = true;

		if(runCode) {
			setInstr(NULL);
			retint = false;
		}
	}

	///////////////
	// Fin de la boucle de l'interpreteur

	// Saute le marqueur de fin de bloc
	if(!oneInstr && *ptr == '}') {
		scriptlog("}\n");
		ptr++;
	} else {
		scriptlog("~;\n");
	}

	return false;
}

/////////////////////////////////////////////////////////////////
// Instructions imperatives

// QC:S
bool ScriptedFunction::instrIf(const bool runCode) {
	bool exprResult;
	char* codeblock;

	///////////
	// Le grand moment de l'implementation du "if"
	//
	if(retint) {
		// Retour d'interruption
		// On recupere l'expression
		exprResult = instr_data.instr_if.exprResult;

		if(exprResult) {
			str_skipblanks(ptr);
			if(instrCmp(ptr, "else")) {
				// Skip else
				if(subRun(false)) {
					assert(false);
					return false;
				}
			}
		}
	} else {
		// Premiere execution, on parse la condition

		// Recherche et elimine la parenthese ouvrante
		str_skipblanks(ptr);
		if(*ptr != '(') {
			syntaxError(ptr, "missing ( after if");
		}
		ptr++;scriptlog("(");

		// Parse la condition
		exprResult = parseExpr(runCode, true);

		scriptlog(")");

		// Stockage du resultat en cas d'interruption
		if(runCode) {
			instr_data.instr_if.exprResult = exprResult;
		}

		// Execution du bloc de code "then"
		if(runCode)
			setInstr("if");

		if(subRun(exprResult && runCode))
			return true;

		str_skipblanks(ptr);
		if(instrCmp(ptr, "else")) {
			// Skip else
			if(subRun(!exprResult && runCode)) {
				return true;
			}
		}
	}

	return false;
}

// QC:S
bool ScriptedFunction::instrFor(const bool runCode) {
	char* codeblock;

	// Les trois membres du for
	char* membre1;
	char* membre2;
	char* membre3;

	assert(retint ? (instr && strcmp(instr, "for") == 0) : true);

	if(retint) {
		// On est en retour d'interruption
		codeblock = instr_data.instr_for[0];
		membre1 = instr_data.instr_for[1];
		membre2 = instr_data.instr_for[2];
		membre3 = instr_data.instr_for[3];

		assert(codeblock);
		assert(membre1);
		assert(membre2);
		assert(membre3);
	} else {
		// Memorise l'instruction
		if(runCode)
			setInstr("for");

		// On se place sur le premier membre du for
		str_skipblanks(ptr);
		if(*ptr != '(') {
			syntaxError(ptr, "missing ( after for");
		}
		ptr++;

		// Memorise le premier membre
		membre1 = ptr;

		// On se place sur le second membre du for
		str_tokens(ptr, ";", true);
		ptr++;

		if(!*ptr) {
			syntaxError(ptr, "for statement incomplete, second member missing.");
		}

		// Memorise le second membre
		membre2 = ptr;

		// On se place sur le troisieme membre
		str_tokens(ptr, ";", true);
		ptr++;

		if(!*ptr) {
			syntaxError(ptr, "for statement incomplete, third member missing.");
		}

		// Memorise le troisieme membre
		membre3 = ptr;

		// Saute la fin du troisieme membre
		parseExpr(false);

		str_skipblanks(ptr);
		if(!*ptr) {
			syntaxError(ptr, "no instruction after for().");
		}

		// Memorise le debut du bloc de code
		codeblock = ptr;
	}

	//////////////////
	// Le grand moment tant attendu,
	// l'implementation du for() !

	if(runCode) {
		if(!retint) {
			// Sauvegarde les pointeurs pour le retour d'interruption
			instr_data.instr_for[0] = codeblock;
			instr_data.instr_for[1] = membre1;
			instr_data.instr_for[2] = membre2;
			instr_data.instr_for[3] = membre3;
		}

		// Reprise du for
		if(retint) {
			if(subRun(runCode))
				return true;
			ptr = membre3;
			parseExpr(runCode);
		}

		for((!retint) ? parseExpr(membre1, runCode, true) : false; parseExpr(membre2, runCode, true); parseExpr(membre3, runCode, true))
		{
			if(subRun(codeblock, runCode))
				return true;
		}

		// Voila, le for() est termine !
		// On saute le bloc de code
		ptr = codeblock;
		setInstr(NULL);
	}
	subRun(false);

	return false;
}

// QC:U (rarement utilise, peu teste)
bool ScriptedFunction::instrDo(const bool runCode) {
	bool isUntil = false;
	char* codeblock = str_skipblanks(ptr);

	if(!retint) {
		// Premiere execution

		// Memorise le debut du bloc de code
		str_skipblanks(ptr);

		// Premiere execution du bloc de code
		if(subRun(runCode))
			return true;

		// Decode l'instruction imperative qui suit (while ou until)
		str_skipblanks(ptr);
		if(instrCmp(ptr, "while")) {
			isUntil = false;
		} else if(instrCmp(ptr, "until")) {
			isUntil = true;
		} else {
			syntaxError(ptr, "no \"while\" or \"until\" after \"do\".");
		}

		if(runCode) {
			instr_data.instr_while.codeblock = codeblock;
			instr_data.instr_while.isUntil = isUntil;
		}
	} else {
		// Retour d'interruption

		codeblock = instr_data.instr_while.codeblock;
		isUntil = instr_data.instr_while.isUntil;

		if(subRun(runCode))
			return true;
	}

	// Saute la parenthese ouvrante de l'expression
	str_skipblanks(ptr);
	if(*ptr != '(') {
		syntaxError(ptr, "missing ( after do");
	}scriptlog("(");
	ptr++;

	if(runCode) {
		setInstr("do");
		char* code;
		char* cond = ptr;
		if(isUntil) {
			while(!parseExpr(ptr = cond, runCode, true)) {
				if(subRun(code = codeblock, runCode))
					return true;
			}
		} else {
			while(parseExpr(ptr = cond, runCode, true)) {
				if(subRun(code = codeblock, runCode))
					return true;
			}
		}
	} else {
		// Saute l'expression
		parseExpr(ptr, runCode);
	}

	return false;
}

// QC:?
bool ScriptedFunction::instrWhile(const bool runCode, bool isUntil) {
	char* condition;
	if(!retint) {
		// Premiere execution

		// Saute la parenthese ouvrante de l'expression
		str_skipblanks(ptr);
		if(*ptr != '(') {
			syntaxError(ptr, "missing ( after while");
		}scriptlog("(");
		ptr++;

		// Marquage du debut de la condition
		condition = ptr;

		// On cherche le bloc de code
		parseExpr(false);
		str_skipblanks(ptr);

		if(runCode) {
			instr_data.instr_while.condition = condition;
			instr_data.instr_while.codeblock = ptr;
			instr_data.instr_while.isUntil = isUntil;
		}
	} else {
		// Retour d'interruption
		condition = instr_data.instr_while.condition;
		isUntil = instr_data.instr_while.isUntil;

		if(subRun(runCode))
			return true;

		if(runCode && retint && *instr_data.instr_while.codeblock == '{' && *ptr == '}')
			ptr++;
	}

	///////////////////
	// Moment un peu moins historique, mais on implemente while/until
	if(runCode) {
		setInstr("while");
		if(isUntil) {
			char* cond;
			char* code;
			while(!parseExpr(cond = condition, runCode, true)) {
				if(subRun(code = ptr, runCode))
					return true;
			}
		} else {
			char* cond;
			char* code;
			while(parseExpr(cond = condition, runCode, true)) {
				if(subRun(code = ptr, runCode))
					return true;
			}
		}
	}

	// Saute le code juste après le dernier test de condition
	subRun(ptr, false);

	return false;
}

// QC:?
bool ScriptedFunction::instrForeach(const bool runCode) {
	if(!retint && *ptr != '(') {
		syntaxError(ptr, "missing ( after foreach");
	}
	ptr++;

	char* &code = instr_data.instr_foreach.codeblock;
	unsigned int &i = instr_data.instr_foreach.current_position;
	char* &val = instr_data.instr_foreach.value_variable;
	char* &key = instr_data.instr_foreach.key_variable;
	ScriptVariable* &tblvar = instr_data.instr_foreach.table;

	if(!retint) {
		i = 0;
		// Parse la variable valeur
		val = str_dupvarname(ptr);
		str_skipblanks(ptr);

		// Parse la valeur cle
		if(*ptr == ',') {
			scriptlog(",");
			ptr++;
			if(!*ptr) {
				return false;
			}
			key = str_dupvarname(ptr);
			str_skipblanks(ptr);
		} else {
			key = NULL;
		}

		if(ptr[0] != 'i' && ptr[1] != 'n') {
			syntaxError(ptr, "missing 'in' keyword in foreach declaration");
		}scriptlog("in");
		ptr += 2;
		if(!*ptr) {
			return false;
		}

		// Parse la table
		tblvar = new ScriptVariable;
		if(subParse(runCode, *tblvar, OP_NUL))
			return true;
		if(!*ptr) {
			return false;
		}
		ptr++;
		str_skipblanks(ptr);

		code = ptr;
	}

	if(runCode) {
		setInstr("foreach");
		ec->registerScriptVariable(val);
		if(key)
			ec->registerScriptVariable(key);

		ScriptTable* tbl = tblvar->gettable();

		while(i < tbl->size()) {
			ScriptVariable* valvar = ec->getVar(val);
			valvar->setval((*tbl)[i]);
			if(key) {
				ec->setval(key, i);
			}
			i++;
			if(subRun(ptr = code, runCode))
				return true;
		}
	} else {
		subRun(ptr, false);
	}

	AOEFREE(val);
	if(key) {
		AOEFREE(key);
	}
	delete tblvar;

	return false;
}

// QC:B
bool ScriptedFunction::instrUsing(const bool runCode) {
	// Recherche et elimine la parenthese ouvrante
	str_skipblanks(ptr);
	if(*ptr != '(') {
		syntaxError(ptr, "missing ( after using");
	}
	ptr++;scriptlog("(");

	ScriptVariable retVal;
	if(subParse(runCode, retVal, OP_NUL))
		return true;
	ptr++; // Consume the closing parenthese

	// Recherche et suppression du ;
	str_skipblanks(ptr);
	if(*ptr != ';') {
		syntaxError(ptr, "missing ; after using");
	}scriptlog(");");
	ptr++; // Consume ;

	// Load the script
	char* newCode = engine()->scriptMap().getScriptCode(VFS::openFile(retVal.getstr(), source()));
	char* ptrsave = ptr;
	ptr = newCode;

	syslog("Interpreting included code");
	bool interrupt = chain(ec);
	// FIXME : chain() destroys some variables used for resuming.
	if(interrupt) {
		// FIXME : This is a hack. Blocking inside using should be possible.
		syntaxError(ptrsave, "Cannot call blocking functions in included files.");
	}

	// Restauration du pointeur d'execution
	ptr = ptrsave;

	return false;
}

// QC:A (memberFunction not implemented)
char* ScriptedFunction::str_dupvarname(char* &ptr, bool memberFunction) {
	bool hasMember = false;
	str_skipblanks(ptr);
	char* left = ptr;
	char* middle;
	int var2namelen = -1;

	str_skipvarname(ptr);

	int varnamelen = ptr - left;
	if(memberFunction) {
		str_skipblanks(ptr);
		if(*ptr == '.') {
			ptr++;
			str_skipblanks(ptr);
			middle = ptr;
			str_skipvarname(ptr);
			var2namelen = ptr - middle;
			hasMember = true;
		}
	}

	char* varname = (char*) AOEMALLOC(varnamelen + var2namelen + 2);
	memcpy(varname, left, varnamelen);
	if(hasMember) {
		varname[varnamelen] = '.';
		memcpy(varname + varnamelen + 1, middle, var2namelen);
	}
	varname[varnamelen + var2namelen + 1] = '\0';
	return varname;
}

// QC:G
void ScriptedFunction::declare_var(const bool runCode) {
	if(runCode) {
		// Recupere le nom de variable
		char* left = ptr;
		char* varname = str_dupvarname(ptr, true);

		// Cree la variable locale
		ec->registerScriptVariable(varname);

		// Libere le nom de la variable
		AOEFREE(varname);

		// Parse l'expression qui suit le mot-cle "var"
		ptr = left;
	}

	if(parseExpr(runCode)) {
		syntaxError(ptr, "Blocking functions don't have a return value.");
	}
}

// QC:S
void ScriptedFunction::declareFunction(const bool runCode, const bool isClass) {
	// Lit les flags
	int flags = (isClass ? FCTFLAG_CLASS : 0);
	bool global = isClass;
	const char* left;
	do {
		str_skipblanks(ptr);
		left = ptr;

		if(instrCmp(ptr, "static")) {
			flags |= FCTFLAG_STATIC;scriptlog("static");
		} else if(instrCmp(ptr, "blocking")) {
			flags |= FCTFLAG_BLOCKING;scriptlog("blocking");
		} else if(instrCmp(ptr, "global")) {
			global = true;
		}
	} while(ptr != left && *ptr);

	// Recupere le nom de la fonction
	str_skipblanks(ptr);
	left = ptr;
	str_skipvarname(ptr);
	if(*ptr == '.') {
		// C'est une fonction membre d'objet
		ptr++;
		str_skipvarname(ptr);
		flags &= ~FCTFLAG_STATIC;
	}

	// Copie le nom de variable dans varname
	int varnamelen = ptr - left;

	char* varname = (char*) alloca(varnamelen + 1);
	strncpy(varname, left, varnamelen);
	varname[varnamelen] = '\0';

	scriptlog(varname);

	// Cree la fonction dans le Contexte

	// Memorise les parametres
	str_skipblanks(ptr);
	char* params = ptr;
	str_tokens(ptr, ")", true); // Trouve la fin des parametres
	ptr++; // Saute la parenthese fermante

	// Stocke le pointeur vers le code source
	str_skipblanks(ptr);
	if(runCode) {
		ScriptVariable *f = (global ? tc : ec)->registerScriptVariable(varname);
		f->settxtfct(ptr, params, flags);
	}

	// Saute le bloc de code
	subRun(false);

	scriptlog("{...}");
}

// QC:S
void ScriptedFunction::declareScriptThread(const bool runCode) {
	int loop = 0;
	int pause = 0;
	const char* left;

	// Parse les flags
	do {
		str_skipblanks(ptr);
		left = ptr;

		if(instrCmp(ptr, "loop")) {
			scriptlog("loop");
			str_skipblanks(ptr);
			if(*ptr == '(') {
				ptr++;

				ScriptVariable v;
				if(subParse(runCode, v, OP_NUL)) {
					syntaxError(ptr, "Blocking functions don't have a return value.");
				}
				if(v.gettype() == VAR_INT) {
					loop = v.getint();
				} else {
					loop = -1;scriptlog("forever");
				}

				ptr++;
				str_skipblanks(ptr);
			} else {
				scriptlog("forever");
				loop = -1;
			}
		}

		if(instrCmp(ptr, "paused") || instrCmp(ptr, "wait") || instrCmp(ptr, "pause")) {
			scriptlog("pause");
			str_skipblanks(ptr);
			if(*ptr == '(') {
				ptr++;

				ScriptVariable v;
				if(subParse(runCode, v, OP_NUL)) {
					syntaxError(ptr, "Blocking functions don't have a return value.");
				}

				if(v.gettype() == VAR_INT) {
					pause = v.getint();
				} else {
					scriptlog("forever");
					pause = -1;
				}

				ptr++;
				str_skipblanks(ptr);
			} else {
				scriptlog("forever");
				pause = -1;
			}
		}
	} while(ptr != left && *ptr);

	if(runCode) {
		char* tname = str_dupvarname(ptr);

		// Creation du thread
		ScriptedFunction* t = new ScriptedFunction(engine(), source(), ptr, ec, NULL, false, false, this);
		t->pause(pause);
		t->loop(loop);

		// Stockage du thread dans la variable
		ec->registerScriptVariable(tname)->setval(t);
		AOEFREE(tname);
	} else {
		str_skipvarname(ptr);
	}

	// Elimine le code
	scriptlog("{...}");
	subRun(false);
}

/////////////////////////////////////////////////////////////////
//    Parser d'expressions

// QC:G
bool ScriptedFunction::parseExpr(char* start, bool runCode, bool returnExpressionResult) {
	ptr = start;
	return parseExpr(runCode, returnExpressionResult);
}

// QC:G
bool ScriptedFunction::parseExpr(bool runCode, bool returnExpressionResult) {
	ScriptVariable retVal;
	bool interrupted = subParse(runCode, retVal, OP_NUL);
	if(interrupted && returnExpressionResult) {
		syntaxError(ptr, "Blocking functions don't have a return value.");
	}
	if(*ptr && *ptr != '}')
		ptr++; // Saute le delimiteur de fin
	return returnExpressionResult ? (retVal.getbool()) : (interrupted);
}

// QC:G
bool ScriptedFunction::subParse(char* newptr, bool runCode, ScriptVariable& retVal, const operatorType opPriority) {
	char* ptrsave = ptr;
	bool rv;

	ptr = newptr;
	rv = subParse(runCode, retVal, opPriority);

	ptr = ptrsave;
	return rv;
}

bool ScriptedFunction::subParse(bool runCode, ScriptVariable& retVal, const operatorType opPriority) {
	str_skipblanks(ptr);

	/////////////////////
	// Expression vide //

	if(!*ptr) {
		// L'expression est vide,
		// donc on renvoie vide !
		if(runCode)
			retVal.setval();
		return false;
	}

	/////////////////////////////////////////////////////////
	// Boucle jusqu'à la fin de l'expression               //
	//
	// ATTENTION :
	// Des return sont caches dans le parsing d'operateurs

	scriptlog("(");

	while(*ptr) {
		///////////////////////////////////////////////////
		// Implementation des expressions sans operateur //

		// Constante numerique
		if(*ptr >= '0' && *ptr <= '9') {
			// Conversion en nombre
			int n = 0;
			while(*ptr >= '0' && *ptr <= '9') {
				n = n * 10 + (*ptr - '0');
				ptr++;
			}

			scriptlog("n="<<n);

			if(runCode)
				retVal.setval(n);
		}

		// Mot-cle ou nom de variable ou nom de fonction
		else if((*ptr >= 'a' && *ptr <= 'z') || (*ptr >= 'A' && *ptr <= 'Z') || *ptr == '_') {
			// true, false et null
			if(instrCmp(ptr, "true")) {
				scriptlog("true");
				if(runCode)
					retVal.setval(1);
			} else if(instrCmp(ptr, "false")) {
				scriptlog("false");
				if(runCode)
					retVal.setval(0);
			} else if(instrCmp(ptr, "null")) {
				scriptlog("null");
				if(runCode)
					retVal.setval();
			}

			// table magic constant
			else if(instrCmp(ptr, "table")) {
				scriptlog("table");
				if(runCode) {
					retVal.setval(new ScriptTable);
				}
			} else if(instrCmp(ptr, "hashtable")) {
				scriptlog("hashtable");
				if(runCode) {
					retVal.setval(new Context(engine()));
				}
			}

			// packing and unpacking
			else if(instrCmp(ptr, "pack")) {
				scriptlog("pack");
				parsePack(runCode, retVal);
			}

			// scope
			else if(instrCmp(ptr, "global") || instrCmp(ptr, "new")) {
				scriptlog("(global)");
				str_skipblanks(ptr);

				char* valueName = str_dupvarname(ptr);

				if(runCode) {
					ScriptVariable* v = tc->getVar(valueName);
					scriptlog(valueName);

					if(v == NULL) {
						// Le nom n'existe pas, on cree une variable nulle
						tc->registerScriptVariable(valueName);
						v = tc->getVar(valueName);scriptlog("(new glob)");
					}

					AOEFREE(valueName);
					retVal.reset();
					retVal.setptr(*v);
				}
			}

			else {
				if(runCode) {
					char* valueName = str_dupvarname(ptr);
					ScriptVariable* v = ec->getVar(valueName);
					if(v == NULL) {
						// On tente dans le topContext
						v = tc->getVar(valueName);
					}

					if(v == NULL) {
						// Aucune variable trouvee. On recherche la forme Class.function
						str_skipblanks(ptr);
						if(*ptr == '.') {
							ptr++;
							const char* functionName;
							STACK_STRDUP(functionName, str_dupvarname(ptr), 4096);
							size_t valueNameLen = strlen(valueName);
							size_t functionNameLen = strlen(functionName);
							valueName = (char*) realloc(valueName, valueNameLen + 1 + functionNameLen + 1);
							valueName[valueNameLen] = '.';
							memcpy(valueName + valueNameLen + 1, functionName, functionNameLen + 1);

							v = ec->getVar(valueName);
							if(!v || v->gettype() != VAR_FCT) {
								// On tente dans le top context
								v = tc->getVar(valueName);
								if(!v || v->gettype() != VAR_FCT) {
									syntaxError(ptr, "Misplaced dot : the variable is not an object or its member is not a function.");
								}
							}
						}

						if(v == NULL) {
							// Le nom n'existe pas, on cree une variable nulle
							v = ec->registerScriptVariable(valueName);scriptlog("(new)");
						}
					}

					scriptlog(valueName);

					AOEFREE(valueName);
					retVal.reset();
					retVal.setptr(*v);
				} else {
					str_skipvarname(ptr);
				}
			}
		}

		///////////////////////////////////
		// Implementation des operateurs //

		operatorType op = findOp(retVal, ptr, opPriority);
		ScriptVariable right;
		char* left = ptr;
		switch(op){
		///////////////////////////
		// Fin d'expression
		case OP_COMMA:
			if(!runCode)
				break;
		case OP_SEMICOLON:
		case OP_BRC_CLOSE:
		case OP_SBR_CLOSE:
		case OP_PAR_CLOSE:
			// Fin d'expression, on quitte ce monde de brutes
			ptr--; // On ne consomme pas le separateur de sortie
			scriptlog("|" << retVal.getint() << ((retVal.gettype()==VAR_STR)?" ":"") << ((retVal.gettype()==VAR_STR)?retVal.getstr():"") );
		case OP_NUL:
			scriptlog(")");
			return false;

		case OP_RUN: // GC:B (destroys resuming context. Do not use within threads)
			scriptlog("#");

			// Parse l'expression qui fournira la chaine
			if(subParse(runCode, right, OP_RUN))
				return true;
			if(!runCode) {
				break;
			}

			if(right.gettype() != VAR_STR) {
				// Probleme, le membre droit n'est pas une chaine
				// On ignore l'operateur # et ce qu'il y a a sa gauche.
				retVal.setval(right);
			} else {
				// Utilisation du membre gauche comme contexte
				Context* op_ec = ec;
				if(retVal.gettype() != VAR_HTBL && retVal.gettype() != VAR_NULL) {
					retVal.setval(new Context(engine()));

					op_ec = retVal.gethashtable();
					if(op_ec) {
						op_ec->setsuper(ec);
					}
				}

				// Execution du membre droit
				char* ptrsave = ptr;
				ptr = right.getstr();
				bool interrupt = chain(op_ec);
				// FIXME : chain() destroys some variables used for resuming.
				if(interrupt) {
					// FIXME : This is a hack. Blocking inside # should be possible.
					syntaxError(ptrsave, "Cannot call blocking functions in # operator's argument.");
				}

				// Restauration du pointeur d'execution
				if(ec == op_ec) {
					ptr = ptrsave;
				}
			}

			break;

			///////////////////////////
			// Affectation Modulo
		case OP_MOD_SET:
			scriptlog("%=");
			if(subParse(runCode, right, OP_MOD_SET))
				return true;

			if(!runCode) {
				break;
			}

			if(retVal.gettype() == VAR_INT && (right.gettype() == VAR_INT || right.gettype() == VAR_PAIR)) {
				retVal.setval(retVal.getint() % right.getint());
			} else if(retVal.gettype() == VAR_PAIR) {
				retVal.setval(retVal.getX() % right.getint(), retVal.getY() % right.getint());
			} else if(retVal.gettype() == VAR_NULL || right.gettype() == VAR_NULL) {
				retVal.setval();
			}

			break;

			///////////////////////////
			// Affectation Division
		case OP_DIV_SET:
			scriptlog("/=");
			if(subParse(runCode, right, OP_DIV_SET))
				return true;

			if(!runCode) {
				break;
			}

			if(retVal.gettype() == VAR_INT && (right.gettype() == VAR_INT || right.gettype() == VAR_PAIR)) {
				if(!right.getint()) {
					syntaxError(ptr, "Divide by zero or by null (maybe a misplaced /)");
				}

				retVal.setval(retVal.getint() / right.getint());
			} else if(retVal.gettype() == VAR_PAIR) {
				if(!right.getint()) {
					syntaxError(ptr, "Divide by zero or by null (maybe a misplaced /)");
				}

				retVal.setval(retVal.getX() / right.getint(), retVal.getY() / right.getint());
			} else if(retVal.gettype() == VAR_NULL || right.gettype() == VAR_NULL) {
				retVal.setval();
			}

			break;

			///////////////////////////
			// Affectation Multiplication
		case OP_MUL_SET:
			scriptlog("*=");
			if(subParse(runCode, right, OP_MUL_SET))
				return true;

			if(!runCode) {
				break;
			}

			if(retVal.gettype() == VAR_INT && (right.gettype() == VAR_INT || right.gettype() == VAR_PAIR)) {
				retVal.setval(retVal.getint() * right.getint());
			} else if(retVal.gettype() == VAR_PAIR) {
				retVal.setval(retVal.getX() * right.getint(), retVal.getY() * right.getint());
			} else if(retVal.gettype() == VAR_NULL || right.gettype() == VAR_NULL) {
				retVal.setval();
			}

			break;

			///////////////////////////
			// Affectation Soustraction
		case OP_SUB_SET:
			scriptlog("-=");
			if(subParse(runCode, right, OP_SUB_SET))
				return true;

			if(!runCode) {
				break;
			}

			if(retVal.gettype() == VAR_INT && (right.gettype() == VAR_INT || right.gettype() == VAR_PAIR)) {
				retVal.setval(retVal.getint() - right.getint());
			} else if(retVal.gettype() == VAR_PAIR && right.gettype() == VAR_PAIR) {
				retVal.setval(retVal.getX() - right.getX(), retVal.getY() - right.getY());
			} else if(retVal.gettype() == VAR_NULL || right.gettype() == VAR_NULL) {
				retVal.setval();
			} else if(retVal.gettype() == VAR_XML) {
				TiXmlElement* elt = retVal.getxml()->ToElement();
				if(elt) {
					if(right.gettype() == VAR_XML) {
						elt->RemoveChild(right.getxml());
					}	else {
						TiXmlNode* child;
						if(right.gettype() == VAR_INT) {
							// Designate subnode by index
							int index = right.getint();
							int i;
							for(i = 0, child = elt->FirstChild(); child && i < index; child = child->NextSibling(), i++);
						} else {
							// Designate subnode by name
							child = elt->FirstChildElement(right.getstr());
						}
						if(child) {
							elt->RemoveChild(child);
						}
					}
				} else {
					syntaxError(ptr, "Cannot delete from a non-element XML node.");
				}
			}

			break;

			///////////////////////////
			// Affectation Addition
		case OP_ADD_SET:
			scriptlog("+=");
			if(subParse(runCode, right, OP_ADD_SET))
				return true;

			if(!runCode) {
				break;
			}

			if(retVal.gettype() == VAR_INT && (right.gettype() == VAR_INT || right.gettype() == VAR_PAIR)) {
				retVal.setval(retVal.getint() + right.getint());
			} else if(retVal.gettype() == VAR_PAIR && right.gettype() == VAR_PAIR) {
				retVal.setval(retVal.getX() + right.getX(), retVal.getY() + right.getY());
			} else if(retVal.gettype() == VAR_NULL || right.gettype() == VAR_NULL) {
				retVal.setval();
			} else if(retVal.gettype() == VAR_XML) {
				TiXmlElement* elt = retVal.getxml()->ToElement();
				if(elt) {
					if(right.gettype() == VAR_XML) {
						elt->InsertEndChild(*(right.getxml()));
					}	else if(right.gettype() == VAR_STR) {
						elt->LinkEndChild(new TiXmlText(right.getstr()));
					}
				} else {
					syntaxError(ptr, "Cannot append to a non-element XML node.");
				}
			}

			break;

			///////////////////////////
			// Set minimum
		case OP_MIN_SET:
			scriptlog("~=");
			if(subParse(runCode, right, OP_MIN_SET))
				return true;

			if(!runCode) {
				break;
			}

			switch(retVal.gettype()){
			case VAR_STR:
				switch(right.gettype()){
				case VAR_INT:
				case VAR_PAIR:
					if(right.getint() < retVal.getint()) {
						retVal.getstr()[right.getint()] = '\0';
					}
					break;
				case VAR_STR: {
					char* subpos = strstr(retVal.getstr(), right.getstr());
					if(subpos != NULL) {
						*subpos = '\0';
					}
				}
					break;
				default:
					break;
				}
				break;

			case VAR_INT:
			case VAR_PAIR:
				switch(right.gettype()){
				case VAR_INT:
				case VAR_PAIR:
					if(retVal.getint() > right.getint()) {
						retVal.setval(right);
					}
					break;

				case VAR_STR:
					if(retVal.getint() < 0) {
						retVal.setval(0);
					} else if(retVal.getint() > right.getint()) {
						retVal.setval(right.getint());
					}

					retVal.setval(retVal.getstr() + right.getint());
					break;

				default:
					retVal.setval(0);
					break;
				}
				break;
			case VAR_XML:
				retVal.getxml()->SetValue(right.getstr());
			default:
				break;
			}
			break;

			///////////////////////////
			// Set maximum
		case OP_MAX_SET:
			scriptlog("^=");
			if(subParse(runCode, right, OP_MIN_SET))
				return true;

			if(!runCode) {
				break;
			}

			switch(retVal.gettype()){
			case VAR_STR:
				switch(right.gettype()){
				case VAR_INT:
				case VAR_PAIR:
					if(right.getint() <= retVal.getint()) {
						memmove(retVal.getstr(), retVal.getstr() + right.getint(), retVal.getint() - right.getint() + 1);
					}
					break;
				case VAR_STR: {
					char* stringPtr = retVal.getstr();
					char* subpos = strstr(stringPtr, right.getstr());
					if(subpos != NULL) {
						// Se place a droite de l'element a supprimer
						subpos += right.getint();

						// Recopie la partie droite dans la chaine
						memmove(stringPtr, subpos, retVal.getint() - (subpos - stringPtr) + 1);
					} else {
						*(retVal.getstr()) = '\0';
					}
				}
					break;
				default:
					break;
				}
				break;

			case VAR_INT:
			case VAR_PAIR:
				switch(right.gettype()){
				case VAR_INT:
				case VAR_PAIR:
					if(retVal.getint() < right.getint()) {
						retVal.setval(right.getint());
					}
					break;

				case VAR_STR:
					if(retVal.getint() < 0) {
						retVal.setval(0);
					} else if(retVal.getint() > right.getint()) {
						retVal.setval(right.getint());
					}

					retVal.getstr()[right.getint()] = '\0';
					break;

				default:
					break;
				}
				break;
			default:
				break;
			}
			break;

			///////////////////////////
			// Affectation
		case OP_SET:
			scriptlog("=");
			if(subParse(runCode, right, OP_SET))
				return true;

			if(!runCode) {
				break;
			}

			right.unlinkPtr();
			retVal.setval(right);

			break;

			///////////////////////////
			// Ou logique
		case OP_OR:
			scriptlog("||");
			{
				bool needRight = true; // short-circuit

				if(runCode && retVal.getbool() == true) {
					retVal.setval(true);
					needRight = false;
				}

				if(subParse(runCode && needRight, right, OP_OR))
					return true;

				if(!runCode) {
					break;
				}

				if(needRight) {
					retVal.setval(right.getbool());
				}
			}

			break;

			///////////////////////////
			// Et logique
		case OP_AND:
			scriptlog("&&");
			{
				bool needRight = true; // short-circuit

				if(runCode && retVal.getbool() == false) {
					retVal.setval(false);
					needRight = false;
				}

				if(subParse(runCode && needRight, right, OP_OR))
					return true;

				if(!runCode) {
					break;
				}

				if(needRight) {
					retVal.setval(right.getbool());
				}
			}

			break;

			///////////////////////////
			// Different
		case OP_NOT_EQUAL:
			scriptlog("!=");
			if(subParse(runCode, right, OP_NOT_EQUAL))
				return true;

			if(!runCode) {
				break;
			}

			// Unifie le type des deux membres
			unifyTypes(retVal, right);

			if(retVal.gettype() == VAR_NULL && right.gettype() == VAR_NULL) {
				retVal.setval(false);
				break;
			}

			if(retVal.gettype() == VAR_INT) {
				// Different
				retVal.setval((retVal.getint() != right.getint()) ? 1 : 0);
			} else if(retVal.gettype() == VAR_STR) {
				// Comparaison de strings
				retVal.setval(strcmp(retVal.getstr(), right.getstr()) ? 1 : 0);
			} else if(retVal.gettype() == VAR_PAIR && right.gettype() == VAR_PAIR) {
				// Comparaison de paires
				retVal.setval((retVal.getX() != right.getX() && retVal.getY() != right.getY()) ? 1 : 0);
			}

			break;

			///////////////////////////
			// Egal
		case OP_EQUALS:
			scriptlog("==");
			if(subParse(runCode, right, OP_EQUALS))
				return true;

			if(!runCode) {
				break;
			}

			// Unifie le type des deux membres
			unifyTypes(retVal, right);

			if(retVal.gettype() == VAR_NULL && right.gettype() == VAR_NULL) {
				retVal.setval(true);
				break;
			}

			if(retVal.gettype() == VAR_INT) {
				// Different
				retVal.setval((retVal.getint() == right.getint()) ? 1 : 0);
			} else if(retVal.gettype() == VAR_STR) {
				// Comparaison de strings
				retVal.setval(strcmp(retVal.getstr(), right.getstr()) ? 0 : 1);
			} else if(retVal.gettype() == VAR_PAIR && right.gettype() == VAR_PAIR) {
				// Comparaison de paires
				retVal.setval((retVal.getX() == right.getX() && retVal.getY() == right.getY()) ? 1 : 0);
			}

			break;

			///////////////////////////
			// Inferieur
		case OP_LT:
			scriptlog("<");
			if(subParse(runCode, right, OP_LT))
				return true;

			if(!runCode) {
				break;
			}

			// Unifie le type des deux membres
			unifyTypes(retVal, right);

			if(retVal.gettype() == VAR_PAIR && right.gettype() == VAR_PAIR) {
				retVal.setval((retVal.getX() < right.getX() && retVal.getY() < right.getY()) ? 1 : 0);
			} else if(retVal.gettype() == VAR_INT || retVal.gettype() == VAR_STR || retVal.gettype() == VAR_PAIR) {
				// Inferieur
				retVal.setval((retVal.getint() < right.getint()) ? 1 : 0);
			}

			break;

			///////////////////////////
			// Inferieur ou egal
		case OP_LT_EQ:
			scriptlog("<=");
			if(subParse(runCode, right, OP_LT_EQ))
				return true;

			if(!runCode) {
				break;
			}

			// Unifie le type des deux membres
			unifyTypes(retVal, right);

			if(retVal.gettype() == VAR_PAIR && right.gettype() == VAR_PAIR) {
				retVal.setval((retVal.getX() <= right.getX() && retVal.getY() <= right.getY()) ? 1 : 0);
			} else if(retVal.gettype() == VAR_INT || retVal.gettype() == VAR_STR || retVal.gettype() == VAR_PAIR) {
				// Inferieur ou egal
				retVal.setval((retVal.getint() <= right.getint()) ? 1 : 0);
			}

			break;

			///////////////////////////
			// Superieur ou egal
		case OP_GT_EQ:
			scriptlog(">=");
			if(subParse(runCode, right, OP_GT_EQ))
				return true;

			if(!runCode) {
				break;
			}

			// Unifie le type des deux membres
			unifyTypes(retVal, right);

			if(retVal.gettype() == VAR_PAIR && right.gettype() == VAR_PAIR) {
				retVal.setval((retVal.getX() >= right.getX() && retVal.getY() >= right.getY()) ? 1 : 0);
			} else if(retVal.gettype() == VAR_INT || retVal.gettype() == VAR_STR || retVal.gettype() == VAR_PAIR) {
				// Different
				retVal.setval((retVal.getint() >= right.getint()) ? 1 : 0);
			}

			break;

			///////////////////////////
			// Superieur
		case OP_GT:
			scriptlog(">");
			if(subParse(runCode, right, OP_GT))
				return true;

			if(!runCode) {
				break;
			}

			// Unifie le type des deux membres
			unifyTypes(retVal, right);

			if(retVal.gettype() == VAR_PAIR && right.gettype() == VAR_PAIR) {
				retVal.setval((retVal.getX() > right.getX() && retVal.getY() > right.getY()) ? 1 : 0);
			} else if(retVal.gettype() == VAR_INT || retVal.gettype() == VAR_STR || retVal.gettype() == VAR_PAIR) {
				// Different
				retVal.setval((retVal.getint() > right.getint()) ? 1 : 0);
			}

			break;

			///////////////////////////
			// Minimum
		case OP_MIN:
			scriptlog("~");
			if(subParse(runCode, right, OP_MIN))
				return true;

			if(!runCode) {
				break;
			}

			retVal.unlinkPtr();

			switch(retVal.gettype()){
			case VAR_STR:
				switch(right.gettype()){
				case VAR_INT:
				case VAR_PAIR:
					if(right.getint() < retVal.getint()) {
						retVal.getstr()[right.getint()] = '\0';
					}
					break;
				case VAR_STR: {
					char* subpos = strstr(retVal.getstr(), right.getstr());
					if(subpos != NULL) {
						*subpos = '\0';
					}
				}
					break;
				default:
					break;
				}
				break;

			case VAR_INT:
			case VAR_PAIR:
				switch(right.gettype()){
				case VAR_INT:
				case VAR_PAIR:
					if(retVal.getint() > right.getint()) {
						retVal.setval(right);
					}
					break;

				case VAR_STR:
					if(retVal.getint() < 0) {
						retVal.setval(0);
					} else if(retVal.getint() > right.getint()) {
						retVal.setval(right.getint());
					}

					retVal.setval(retVal.getstr() + right.getint());
					break;

				default:
					retVal.setval(0);
					break;
				}
				break;
			default:
				break;
			}
			break;

			///////////////////////////
			// Maximum
		case OP_MAX:
			scriptlog("^");
			if(subParse(runCode, right, OP_MIN))
				return true;

			if(!runCode) {
				break;
			}

			retVal.unlinkPtr();

			switch(retVal.gettype()){
			case VAR_STR:
				switch(right.gettype()){
				case VAR_INT:
				case VAR_PAIR:
					if(right.getint() < retVal.getint()) {
						memmove(retVal.getstr(), retVal.getstr() + right.getint(), retVal.getint() - right.getint() + 1);
					}
					break;
				case VAR_STR: {
					char* stringPtr = retVal.getstr();
					char* subpos = strstr(stringPtr, right.getstr());
					if(subpos != NULL) {
						// Se place a droite de l'element a supprimer
						subpos += right.getint();

						// Recopie la partie droite dans la chaine
						memmove(stringPtr, subpos, retVal.getint() - (subpos - stringPtr) + 1);
					} else {
						*(retVal.getstr()) = '\0';
					}
				}
					break;
				default:
					break;
				}
				break;

			case VAR_INT:
			case VAR_PAIR:
				switch(right.gettype()){
				case VAR_INT:
				case VAR_PAIR:
					if(retVal.getint() < right.getint()) {
						retVal.setval(right);
					}
					break;

				case VAR_STR:
					if(retVal.getint() < 0) {
						retVal.setval(0);
					} else if(retVal.getint() > right.getint()) {
						retVal.setval(right.getint());
					}

					retVal.getstr()[right.getint()] = '\0';
					break;

				default:
					break;
				}
				break;
			default:
				break;
			}
			break;

			///////////////////////////
			// Soustraction
		case OP_SUB:
			scriptlog("!-");
			if(subParse(runCode, right, OP_ADD))
				return true;

			if(!runCode) {
				break;
			}

			// Unifie le type des deux membres
			unifyTypes(retVal, right);

			if(retVal.gettype() == VAR_INT) {
				// Soustraction
				retVal.setval(retVal.getint() - right.getint());
			} else if(retVal.gettype() == VAR_PAIR) {
				if(right.gettype() == VAR_PAIR) {
					retVal.setval(retVal.getX() - right.getX(), retVal.getY() - right.getY());
				} else if(right.gettype() == VAR_INT) {
					retVal.setval(retVal.getX() - right.getint(), retVal.getY() - right.getint());
				}
			}

			break;

			///////////////////////////
			// Addition
		case OP_ADD:
			scriptlog("+");
			if(subParse(runCode, right, OP_ADD))
				return true;

			if(!runCode) {
				break;
			}

			// Unifie le type des deux membres
			unifyTypes(retVal, right);

			if(retVal.gettype() == VAR_INT) {
				// Addition
				retVal.setval(retVal.getint() + right.getint());
			} else if(retVal.gettype() == VAR_STR) {
				// Concatenation de strings
				retVal.concat(right.getstr());
			} else if(retVal.gettype() == VAR_PAIR) {
				if(right.gettype() == VAR_PAIR) {
					retVal.setval(retVal.getX() + right.getX(), retVal.getY() + right.getY());
				} else if(right.gettype() == VAR_INT) {
					retVal.setval(retVal.getX() + right.getint(), retVal.getY() + right.getint());
				}
			} else {
				syntaxError(ptr, "Cannot add objects of these types.");
			}

			break;

			///////////////////////////
			// Modulo
		case OP_MOD:
			scriptlog("%");
			if(subParse(runCode, right, OP_MUL))
				return true;

			if(!runCode) {
				break;
			}

			// Unifie le type des deux membres
			unifyTypes(retVal, right);

			if(retVal.gettype() == VAR_INT) {
				// Modulo
				retVal.setval(retVal.getint() % right.getint());
			} else if(retVal.gettype() == VAR_PAIR) {
				retVal.setval(retVal.getX() % right.getint(), retVal.getY() % right.getint());
			}

			break;

			///////////////////////////
			// Division
		case OP_DIV:
			scriptlog("/");
			if(subParse(runCode, right, OP_MUL))
				return true;

			if(!runCode) {
				break;
			}

			if(retVal.gettype() == VAR_INT) {
				if(!right.getint()) {
					syntaxError(ptr, "Divide by zero or by null (maybe a misplaced /)");
				}

				// Division
				retVal.setval(retVal.getint() / right.getint());
			} else if(retVal.gettype() == VAR_PAIR) {
				if(!right.getint()) {
					syntaxError(ptr, "Divide by zero or by null (maybe a misplaced /)");
				}

				retVal.setval(retVal.getX() / right.getint(), retVal.getY() / right.getint());
			}

			break;

			///////////////////////////
			// Multiplication
		case OP_MUL:
			scriptlog("*");
			if(subParse(runCode, right, OP_MUL))
				return true;

			if(!runCode) {
				break;
			}

			// Unifie le type des deux membres
			unifyTypes(retVal, right);

			if(retVal.gettype() == VAR_INT) {
				// Multiplication
				retVal.setval(retVal.getint() * right.getint());
			} else if(retVal.gettype() == VAR_PAIR) {
				retVal.setval(retVal.getX() * right.getint(), retVal.getY() * right.getint());
			}

			break;

			///////////////////////////
			// Negation logique
		case OP_NOT:
			scriptlog("!");
			if(subParse(runCode, right, OP_NOT))
				return true;

			if(!runCode) {
				break;
			}

			// Negation logique
			retVal.setval(right.getbool() ? 0 : 1);

			break;

			///////////////////////////
			// Oppose
		case OP_NEG:
			scriptlog("-");
			if(subParse(runCode, right, OP_NEG))
				return true;

			if(!runCode) {
				break;
			}

			// Oppose
			if(right.gettype() == VAR_INT) {
				retVal.setval(-right.getint());
			} else if(right.gettype() == VAR_PAIR) {
				retVal.setval(-right.getX(), -right.getY());
			}

			break;

			///////////////////////////
			// Decrementation
		case OP_DEC:
			scriptlog("--");
			if(subParse(runCode, right, OP_DEC))
				return true;

			if(!runCode) {
				break;
			}

			if(retVal.gettype() == VAR_INT && right.gettype() == VAR_NULL) {
				// Post-decrementation
				int tmp = retVal.getint();
				retVal.setval(tmp - 1);
				retVal.unlinkPtr();
				retVal.setval(tmp);
			} else if(retVal.gettype() == VAR_NULL && right.gettype() == VAR_INT) {
				// Pre-decrementation
				right.setval(right.getint() - 1);
				retVal.setval(right.getint());
			}

			break;

			///////////////////////////
			// Incrementation
		case OP_INC:
			scriptlog("++");
			if(subParse(runCode, right, OP_INC))
				return true;

			if(!runCode) {
				break;
			}

			if(retVal.gettype() == VAR_INT && right.gettype() == VAR_NULL) {
				// Post-incrementation
				int tmp = retVal.getint();
				retVal.setval(tmp + 1);
				retVal.unlinkPtr();
				retVal.setval(tmp);
			} else if(retVal.gettype() == VAR_NULL && right.gettype() == VAR_INT) {
				// Pre-incrementation
				right.setval(right.getint() + 1);
				retVal.setval(right.getint());
			}

			break;

			///////////////////////////
			// Paire
		case OP_PAIR:
			scriptlog(":");
			if(runCode && retVal.gettype() == VAR_PAIR) {
				// We are parsing a rect
				// FIXME : Could parse it better.
				if(subParse(runCode, right, OP_SBR))
					return true;
				if(right.gettype() != VAR_PAIR) {
					syntaxError(ptr, "Malformed rectangle declaration. Please check operator precedence.");
				}
				{
					Point offset(retVal.getPoint());
					Point size(right.getPoint());
					retVal.reset();
					retVal.setval(offset, size);
				}
			} else {
				if(subParse(runCode, right, OP_MUL))
					return true;
				if(!runCode) {
					break;
				}

				{

					int px = retVal.getint();
					int py = right.getint();
					retVal.reset();
					retVal.setval(px, py);
				}
			}
			break;

			/////////////////////////////
			// Crochet (table)
		case OP_SBR:
			scriptlog("[");
			{
				if(subParse(runCode, right, OP_MUL))
					return true;

				if(*ptr == ']') {
					ptr++;scriptlog("]");
				} else {
					// Erreur optionnelle
					syntaxError(ptr, "missing closing square bracket");
				}

				if(!runCode) {
					break;
				}

				if(retVal.gettype() == VAR_TBL) {
					unsigned int index = (unsigned int) (right.getint());

					ScriptTable* t = retVal.gettable();
					if(t->size() <= index) {
						t->resize(index + 1, ScriptVariable());
						syslog("Increasing table size to %d", index + 1);
					}
					retVal.reset();
					retVal.setptr((*t)[index]);
				} else if(retVal.gettype() == VAR_XML) {
					// Access to child
					TiXmlNode* parent = retVal.getxml();
					TiXmlNode* child = NULL;
					if(right.gettype() == VAR_INT) {
						// Designate subnode by index
						int index = right.getint();
						int i;
						for(i = 0, child = parent->FirstChild(); child && i < index; child = child->NextSibling(), i++);
					} else {
						// Designate subnode by name
						child = parent->FirstChildElement(right.getstr());
					}
					retVal.reset();
					retVal.setval(child);
				} else {
					Context* c;
					if(retVal.gettype() != VAR_HTBL) {
						retVal.setval(new Context(engine()));
					}
					c = retVal.gethashtable();

					retVal.reset();

					char* varname = right.getstr();

					if(varname != NULL && *varname) {
						ScriptVariable* v = c->getVar(varname);
						if(v == NULL) {
							v = c->registerScriptVariable(varname);
						}
						retVal.setptr(*v);
					} else {
						// Pas de variable, on retourne la liste.
						ScriptVariable vlist;
						c->getVarlist(vlist, false);
						retVal.setval(vlist);
					}
				}

			}
			break;

			/////////////////////////////
			// Parentheses
		case OP_PAR:
			if(retVal.gettype() == VAR_FCT || retVal.gettype() == VAR_TXTFCT) {
				scriptlog("/(");
				// Les parentheses sont en fait un appel de fonction
				// Accrochez-vous, ce code est complique
				//
				// NOTE : Dans ce bloc de code, runCode == true

				Context* params = new Context(engine()); // Parametres

				// Lecture des parametres
				char* wantedParams = retVal.getparams();
				if(*wantedParams != '(') {
					syntaxError(wantedParams, "wantedParams malformed.");
				}

				str_skipblanks(ptr);

				ScriptVariable* thisVar = params->registerScriptVariable("this");
				if(!(retVal.getint() & FCTFLAG_STATIC)) {
					thisVar->setval(lastObject);
				}

				while(*ptr && *ptr != ')') {
					// Saute le separateur
					wantedParams++;
					str_skipblanks(wantedParams);
					if(*wantedParams == '&') {
						// Skip address indicator
						wantedParams++;
					}

					// Lit le nom du parametre
					char* left = wantedParams;
					str_skipvarname(wantedParams);
					char* pName = (char*) alloca(wantedParams - left + 1);
					memcpy(pName, left, wantedParams - left);
					pName[wantedParams - left] = '\0';
					str_skipblanks(wantedParams);

					// pName contient le nom du parametre

					if(*wantedParams != ',' && *wantedParams != ')') {
						syntaxError(ptr, "error while parsing function arguments");
					}

					// Lit la valeur du parametre
					ScriptVariable pValue;
					if(subParse(runCode, pValue, OP_NUL))
						xerror(false, "Cannot block in function parameters.");
					str_skipblanks(ptr);

					// Associe le nom du parametre a sa valeur
					pValue.unlinkPtr();
					params->setval(pName, pValue);

					if(*ptr != ',' && *ptr != ')') {
						syntaxError(ptr, "error while parsing function arguments' values");
					}

					// On doit avoir le meme marqueur de fin d'expression donc le meme nombre de parametres
					if(*wantedParams != *ptr) {
						syntaxError(ptr, "wrong number of parameters");
					}

					if(*ptr == ',') {
						scriptlog(",");
						ptr++;
					}
				}

				if(*ptr != ')') {
					syntaxError(ptr, "big problem with function argument parsing");
				}

				// Saute la parenthese fermante
				if(*ptr) {
					ptr++;scriptlog(")/");
				}

				// Gestion des fonctions bloquantes
				if(retVal.gettype() == VAR_FCT && (retVal.getint() & FCTFLAG_BLOCKING) == FCTFLAG_BLOCKING) {
					setInstr("fct");

					instr_data.instr_fct.fctVar = new ScriptVariable(retVal);
					instr_data.instr_fct.fctParams = params;
					instr_data.instr_fct.fctPtr = ptr;

					// Fonction native
					retVal.callNativeFunction(params);
					if(retVal.queryFunctionBlock(params))
						return true;
				} else {
					// Appel de la fonction
					if(retVal.gettype() == VAR_TXTFCT) {
						// Fonction texte
						// Equivalente a un bloc de code {...} sauf que
						// le supercontexte est tc au lieu de ec

						bool isClass = retVal.getint() & FCTFLAG_CLASS;
						assert(isClass ? !(retVal.getint() & FCTFLAG_BLOCKING) : true);

						if(thisVar->gettype() == VAR_NULL) {
							thisVar->reset();
							thisVar->setval(ec);
						}

						char* subptr = retVal.getstr();

						if(retVal.getint() & FCTFLAG_BLOCKING) {
							setInstr("txtfct");
							instr_data.instr_fct.fctPtr = ptr;
							instr_data.instr_fct.fctParams = params;
							instr_data.instr_fct.fctVar = new ScriptVariable(retVal);
							instr_data.instr_fct.htbl = NULL;
						}

						if(isClass) {
							lastObject.reset();

							Context* instance = new Context(engine());
							instance->setsuper(params);
							subFunctionRun(subptr, runCode, instance, true);
							retVal.reset();
							retVal.setval(instance);
						} else {
							if(lastObject.gettype() == VAR_HTBL) {
								lastObject.reset();

								Context* &oldsuper = instr_data.instr_fct.oldsuper;
								Context* &htbl = instr_data.instr_fct.htbl;
								htbl = thisVar->gethashtable();
								assert(htbl);
								params->setsuper(htbl);
								oldsuper = htbl->getsuper();
								htbl->setsuper(tc);

								if(subFunctionRun(subptr, runCode, params))
									return true;

								retVal = *(params->getVar("@retval"));
								htbl->setsuper(oldsuper);
							} else {
								lastObject.reset();

								params->setsuper(tc);
								if(subFunctionRun(subptr, runCode, params))
									return true;
								retVal = *(params->getVar("@retval"));
							}
						}

						// instruction return
						scriptlog("rval="<<retVal.getint());
					} else {
						// Fonction native

						// Appel de la fonction
						retVal.callNativeFunction(params);

						// Renvoie la valeur de retour
						retVal = *(params->getVar("@retval"));
					}
				}

				// Appel termine, on libere les parametres
				if(!instr)
					delete params;
				setInstr(NULL);
				lastObject.reset();
			} else {
				scriptlog("(");

				// Les parentheses sont de simples parentheses et non un appel de fonction
				retVal.setval();
				if(subParse(runCode, retVal, OP_NUL))
					return true;
				if(*ptr == ')') {
					ptr++;scriptlog(")");
				} else {
					// Erreur optionnelle
					syntaxError(ptr, "missing closing parenthese");
				}
			}
			break;

			/////////////////////////////
			// Evaluation de chaine
		case OP_EVAL:
			scriptlog("$");

			// Parse l'expression qui fournira la chaine
			if(subParse(runCode, right, OP_EVAL))
				return true;
			if(!runCode) {
				break;
			}

			if(right.gettype() != VAR_STR) {
				// Probleme, le membre droit n'est pas une chaine
				// On ignore l'operateur $ et ce qu'il y a a sa gauche.
				retVal.setval(right);
			} else {
				if(retVal.gettype() == VAR_INT || retVal.gettype() == VAR_STR) {
					retVal.concat(right.getstr());
					right.setval(retVal.getstr());
				}
				char* subptr = right.getstr();
				if(subParse(subptr, runCode, retVal, OP_NUL))
					return true;
			}

			break;

			/////////////////////////////
			// Lecture de fichier
		case OP_FILE:
			scriptlog("@");

			// Parse l'expression qui fournira la chaine
			if(subParse(runCode, right, OP_FILE))
				return true;

			if(!runCode) {
				break;
			}

			if(right.gettype() != VAR_STR && right.gettype() != VAR_INT) {
				// Probleme, le membre droit n'est pas une chaine
				// On ignore l'operateur @ et ce qu'il y a a sa gauche.
				retVal.setval("");
			} else {
				if(retVal.gettype() == VAR_INT || retVal.gettype() == VAR_STR) {
					retVal.concat(right.getstr());
					right.setval(retVal.getstr());
				}
				VFS::File* f = VFS::openFile(right.getstr(), source());
				char* buf = f->readToString();

				// Sauvegarde la chaine dans la valeur de retour
				retVal.setval(buf);

				AOEFREE(buf);
				delete f;
			}
			break;

			/////////////////////////////
			// Reference objet
		case OP_DOT:
			scriptlog(".");
			str_skipblanks(ptr);
			if(runCode) {
				if(retVal.gettype() == VAR_OBJ) {
					// Build the final name : "className.functionName"
					lastObject = retVal;
					const char* className = lastObject.getobj()->getClassName();
					char* functionName = str_dupvarname(ptr);
					scriptlog(functionName);

					// Allocate memory for the name.
					size_t classNameLen = strlen(className);
					size_t functionNameLen = strlen(functionName);
					// Prevent too long names.
					assert(classNameLen + functionNameLen < 4096);
					char* finalName = (char*) alloca(classNameLen + 1 + functionNameLen + 1);

					memcpy(finalName, className, classNameLen);
					finalName[classNameLen] = '.';
					memcpy(finalName + classNameLen + 1, functionName, functionNameLen);
					finalName[classNameLen + 1 + functionNameLen] = '\0';
					AOEFREE(functionName);

					ScriptVariable* fct = ec->getVar(finalName);
					if(!fct) {
						fct = tc->getVar(finalName);
						if(!fct) {
							syntaxError(ptr, "Function does not exist.");
						}
					}
					retVal = *(fct);
				} else if(retVal.gettype() == VAR_PAIR) {
					if(*ptr == 'x' || *ptr == 'X') {
						scriptlog("x");
						retVal.unlinkPtr();
						retVal.setval(retVal.getX());
						ptr++;
					} else if(*ptr == 'y' || *ptr == 'Y') {
						scriptlog("y");
						retVal.unlinkPtr();
						retVal.setval(retVal.getY());
						ptr++;
					} else {
						syntaxError(ptr, "Pairs only have X and Y members");
					}
				} else if(retVal.gettype() == VAR_HTBL) {
					// subparse with the hashtable as context
					instr_data.instr_dot.currentContext = ec;
					lastObject = retVal;
					retVal.reset();
					ec = lastObject.gethashtable();
					setInstr("dot");
					if(subParse(runCode, retVal, OP_DOT + 1))
						return true;
					setInstr(NULL);
				} else if(retVal.gettype() == VAR_XML) {
					TiXmlElement* elt = retVal.getxml()->ToElement();
					if(!elt) {
						syntaxError(ptr, "This is not an XML element, it does not have attributes.");
					}
					char* attrName = str_dupvarname(ptr);
					scriptlog(attrName);
					str_skipblanks(ptr);
					retVal.reset();
					if(ptr[0] == '=' && ptr[1] != '=') {
						// Set attribute
						// Parse l'expression qui fournira la valeur
						if(subParse(runCode, retVal, OP_NUL)) {
							syntaxError(ptr, "Cannot block in expressions.");
							AOEFREE(attrName);
							return true;
						}
						char* value = retVal.getstr();
						if(value) {
							elt->SetAttribute(attrName, value);
							retVal.setval(value);
						} else {
							elt->RemoveAttribute(attrName);
							// Leave retVal at NULL
						}
					} else {
						// Return attribute value
						retVal.setval(elt->Attribute(attrName));
					}
					AOEFREE(attrName);
				} else if(retVal.gettype() == VAR_TBL) {
					if(instrCmp(ptr, "push")) {
						scriptlog("push");
						bool back;
						str_skipblanks(ptr);
						if(instrCmp(ptr, "back")) {
							scriptlog(" back");
							back = true;
						} else if(instrCmp(ptr, "front")) {
							scriptlog(" front");
							back = false;
							syslog("WARNING: 'table.push front' is slow if the table is big.");
						} else {
							syntaxError(ptr, "Table : missing 'front' or 'back' keyword.");
							back = false;
						}
						ScriptTable* s = retVal.gettable();

						right.reset();
						if(subParse(runCode, right, OP_MUL))
							return true;

						if(back) {
							s->push_back(right);
						} else {
							s->insert(s->begin(), right);
						}
					} else if(instrCmp(ptr, "pop")) {
						str_skipblanks(ptr);scriptlog("push");
						bool back = false;
						str_skipblanks(ptr);
						if(instrCmp(ptr, "back")) {
							scriptlog(" back");
							back = true;
						} else if(instrCmp(ptr, "front")) {
							scriptlog(" front");
							syslog("WARNING: 'table.pop front' is slow if the table is big.");
						} else {
							syntaxError(ptr, "Table : missing 'front' or 'back' keyword.");
						}
						ScriptTable* s = retVal.gettable();

						if(back) {
							retVal = s->back();
							s->pop_back();
						} else {
							retVal = s->front();
							s->erase(s->begin());
						}
					} else if(instrCmp(ptr, "clone")) {
						ScriptVariable tmp(retVal);
						retVal.setval(tmp); // XXX : Test me.
					} else if(instrCmp(ptr, "size")) {
						ScriptTable* t = retVal.gettable();
						if(t) {
							retVal.reset();
							retVal.setval((int) (t->size()));
						} else {
							retVal.reset();
						}
					} else {
						syntaxError(ptr, "Table: unknown operation.");
					}
				} else {
					syntaxError(ptr, "trying to reference a member of a non-object (misplaced dot)");
				}
			}
			break;

			/////////////////////////////
			// Quotes
		case OP_STRING:
			scriptlog("\"");
			while(*ptr && *ptr != '"') {
				if(*ptr == '\\') {
					ptr++;
					if(!*ptr) {
						syntaxError(ptr, "there is something wicked around a \\ in a quoted string litteral");
					}
				}
				ptr++;
			}

			// Retourne la chaine
			if(runCode) {
				// Copie les caracteres entre left et ptr dans str
				char* str = (char*) alloca(ptr - left + 1);

				char* src = left;
				char* dst = str;
				while(src != ptr) {
					if(*src == '\\') {
						src++;
						if(*src == 'n') {
							*(dst++) = '\n';
							src++;
						} else {
							*(dst++) = *(src++);
						}
					} else {
						*(dst++) = *(src++);
					}
				}

				*dst = '\0';scriptlog(str);
				retVal.setval(str);
			}

			// Saute les guillemets de fin si la chaine n'est pas terminee
			if(*ptr) {
				ptr++;scriptlog("\"");
			}

			break;

		}

		if(runCode && op != OP_DOT) {
			lastObject.reset();
		}

		str_skipblanks(ptr);
	}

	return false;
}

// QC:S
operatorType ScriptedFunction::findOp(ScriptVariable &retVal, char* &ptr, operatorType opPriority) // retVal sert a distinguer le - unaire du - binaire
{
	operatorType found;

	str_skipblanks(ptr);

	do {
		char* opStart = ptr;

		found = op2OpType(ptr);

		// Traite le cas particulier du moins unaire/binaire
		if(found == OP_NEG) {
			if(retVal.gettype() != VAR_NULL) {
				// On a un moins binaire puisqu'il y a une valeur a gauche
				found = OP_SUB;
			}
		}

		if(found <= opPriority) {
			// Un operateur non-prioritaire est considéré comme nul et non consomme.
			ptr = opStart;
			return OP_NUL;
		}
	} while(!found && *(++ptr));

	return found;
}

// QC:G (extremement sollicite, doit etre tres performant. Ne fonctionne qu'en ASCII)
operatorType ScriptedFunction::op2OpType(char* &srcop) {
	char op[2] = { ' ', ' ' }; // Operateur nul par defaut

	// Test du premier caractere
	if((srcop[0] >= '!' && srcop[0] <= '/') || (srcop[0] >= ':' && srcop[0] <= '@') || (srcop[0] >= '[' && srcop[0] <= '^')
	|| (srcop[0] >= '{' && srcop[0] <= '~'))
	{
		op[0] = srcop[0]; // C'est bien un caractere d'operateur
	}

	// Le premier caractere ne peut pas faire partie d'un operateur : on abandonne
	if(op[0] == ' ') {
		return OP_NUL;
	}

	// Test du second caractere
	if((srcop[1] >= '!' && srcop[1] <= '/') || (srcop[1] >= ':' && srcop[1] <= '@') || (srcop[1] >= '[' && srcop[1] <= '^')
	|| (srcop[1] >= '{' && srcop[1] <= '~'))
	{
		op[1] = srcop[1]; // C'est bien un caractere d'operateur
	}

	// On essaie avec les operateurs a 2 caracteres
	for(operatorType opType = 0; opType < OP_COUNT; opType++) {
		// En coupe-circuit, pour eviter les tests inutiles et les boucles superflues !
		if(op[0] == opList[opType][0] && op[1] == opList[opType][1]) {
			if(op[1] == ' ')
				srcop += 1;
			else
				srcop += 2;

			return opType;
		}
	}

	// On essaie avec les operateurs a 1 caractere (dont le 2eme caractere est un espace)
	for(operatorType opType = 0; opType < OP_COUNT; opType++) {
		// En coupe-circuit, pour eviter les tests inutiles et les boucles superflues !
		if(op[0] == opList[opType][0] && opList[opType][1] == ' ') {
			srcop += 1;
			return opType;
		}
	}

	// Meme apres des efforts considerables, il ne s'agit pas d'un operateur !
	return OP_NUL;
}

// Unifie le type des deux variables.
// QC:A (ne fuit pas, c'est deja bien)
void ScriptedFunction::unifyTypes(ScriptVariable& l, ScriptVariable& r) {
	l.unlinkPtr();
	r.unlinkPtr();

	varType lt = l.gettype();
	varType rt = r.gettype();

	// Si la variable de gauche est nulle uniquement, on echange les variables.
	if(lt == VAR_NULL && rt != VAR_NULL) {
		l = r;
		lt = rt;
		r.setval();
		rt = VAR_NULL;
	}


	// Si l'une des deux est une string, les deux sont transformees en string.
	if(lt == VAR_STR && rt != VAR_STR) {
		r.setval(rt == VAR_NULL?"":r.getstr());
		rt = VAR_STR;
	} else if(lt != VAR_STR && rt == VAR_STR) {
		l.setval(lt == VAR_NULL?"":l.getstr());
		lt = VAR_STR;
	}

	// Echange de variables pour avoir des types connus a gauche.
	if(lt != VAR_STR && rt == VAR_PAIR && lt != VAR_PAIR) {
		ScriptVariable swaptmp(r);
		r = l;
		l = swaptmp;
		lt = l.gettype();
		rt = r.gettype();
	}

	// Si l'une des deux est un int, les deux sont transformees en int.
	if(lt == VAR_INT && rt != VAR_INT) {
		r.setval(0);
		rt = VAR_INT;
	}
}

// QC:G
void ScriptedFunction::parsePack(bool runCode, ScriptVariable& retVal) {
	// ptr is right after the pack keyword.
	str_skipblanks(ptr);

	// parse parentheses
	if(*ptr != '(') {
		syntaxError(ptr, "pack: Missing opening parenthese.");
		return;
	}

	// Saute la parenthese
	ptr++;
	str_skipblanks(ptr);

	ScriptVariable pv;

	if(!runCode) {
		// Quickly skip any parameter, without check.
		str_skipblanks(ptr);
		while(*ptr == ',') {
			ptr++;
			if(subParse(runCode, pv, OP_NUL))
				return;
			str_skipblanks(ptr);
		}
		assert(*ptr == ')');
		ptr++;

		return;
	}

	ScriptVariable formatStringVar;
	if(subParse(runCode, formatStringVar, OP_NUL))
		return;
	str_skipblanks(ptr);

	const char* format = formatStringVar.getstr();
	if(!format || !*format) {
		syntaxError(ptr, "pack: Valid format string expected.");
		return;
	}

	unsigned long bufferLen = computePackedStructureLength(format);
	syslog("Packing variables with format %s.", format);

	char* buffer = (char*) AOEMALLOC(bufferLen);
	char* currentValue = buffer;
	bool isSized = false;

	if(*format == '!') {
		format++;
		currentValue += 4;
		isSized = true;

		syslog("pack: emitting sized structure.");
	}

	do {
		str_skipblanks(ptr);
		if(*ptr != ',') {
			syntaxError(ptr, "pack: Comma expected. Not enough parameters.");
			return;
		}
		ptr++;
		if(subParse(runCode, pv, OP_NUL))
			return;

		switch(*format){
		case 'i': {
			*((int32_t*) (currentValue)) = HTON_INT32(pv.getint());
			currentValue += 4;
		}
			break;

		case 'I': {
			*((uint32_t*) (currentValue)) = HTON_UINT32((unsigned int)pv.getint());
			currentValue += 4;
		}
			break;

		case 'c': {
			*((int8_t*) (currentValue)) = HTON_INT8((char)pv.getint());
			currentValue += 1;
		}
			break;

		case 'C': {
			*((uint8_t*) (currentValue)) = HTON_UINT8((unsigned char)pv.getint());
			currentValue += 1;
		}
			break;

		case 'b': {
			syntaxError(ptr, "Byte array unsupported in script engine.");
		}
			break;

		case 's': {
			char* srcString = pv.getstr();
			assert(srcString);

			if(isSized) {
				unsigned long stringLen = strlen(srcString) + 1;
				bufferLen += stringLen;
				unsigned long currentPosition = currentValue - buffer;
				buffer = (char*) realloc(buffer, bufferLen);
				currentValue = buffer + currentPosition;
				memcpy(currentValue, srcString, stringLen);
				currentValue += stringLen;
			} else {
				strncpy(currentValue, srcString, 256);
				currentValue += 256;
			}
		}
			break;

		case 'S': {
			if(pv.gettype() != VAR_PACK) {
				syntaxError(ptr, "pack: Packed structure expected.");
				return;
			}
			const char* subFormat = pv.getparams();
			const char* subBuffer = (const char*) pv.getdata();

			bool subSized = false;
			if(*subFormat == '!') {
				subSized = true;
			}

			if(!isSized) {
				syntaxError(ptr,
				            "pack: Cannot put a sub-structure in a unsized structure (Prefix the format string with an exclamation point)");
				return;
			}

			unsigned long size = pv.getint();
			if(format[1] == '\0' && subSized) {
				// The sub structure is the last member.
				// Storing size in the sub structure is not needed because we can deduce it easily while unpacking.
				size -= 4;
				subBuffer += 4;
			}

			bufferLen += size;
			unsigned long currentPosition = currentValue - buffer;
			buffer = (char*) realloc(buffer, bufferLen);
			currentValue = buffer + currentPosition;
			memcpy(currentValue, subBuffer, size);
			currentValue += size;
		}
			break;

		default: {
			syntaxError(ptr, "Unsupported format string.");
		}
			break;
		}

	} while(*(++format));

	if(runCode && isSized) {
		*((uint32_t*) (buffer)) = HTON_UINT32(currentValue - buffer);
	}

	str_skipblanks(ptr);
	if(*ptr != ')') {
		syntaxError(ptr, "pack: Missing closing parenthese.");
		return;
	}
	ptr++;

	if(runCode)
		retVal.setval(formatStringVar.getstr(), buffer);
}

// QC:G
void ScriptedFunction::parseUnpack(bool runCode) {
	/* unpack example:
	 *  unpack(packedStructure, intvar, strvar); // Returns the number of variables extracted. All variables are allocated in the current context.
	 */

	// ptr is right after the pack keyword.
	str_skipblanks(ptr);

	// parse parentheses
	if(*ptr != '(') {
		syntaxError(ptr, "pack: Missing opening parenthese.");
		return;
	}

	// Saute la parenthese
	ptr++;
	str_skipblanks(ptr);

	ScriptVariable pv;

	if(!runCode) {
		// Quickly skip any parameter, without check.
		str_skipblanks(ptr);
		while(*ptr == ',') {
			ptr++;
			if(subParse(runCode, pv, OP_NUL))
				return;
			str_skipblanks(ptr);
		}
		assert(*ptr == ')');
		ptr++;

		return;
	}

	// Parse source structure
	if(subParse(runCode, pv, OP_NUL))
		return;

	if(pv.gettype() != VAR_PACK) {
		syntaxError(ptr, "unpack: First parameter is not a packed structure.");
		return;
	}

	const char* format = pv.getparams();
	const char* buffer = pv.getdata();
	const char* currentValue = buffer;
	unsigned long bufferLen = 0;
	bool isSized;

	if(*format == '!') {
		format++;
		bufferLen = NTOH_UINT32(*((uint32_t*) (currentValue)));
		currentValue += 4;
		isSized = true;
	}

	do {
		// Parse destination variable name
		str_skipblanks(ptr);
		if(*ptr != ',') {
			syntaxError(ptr, "unpack: Need more parameters to unpack this structure. Waiting for ','.");
			return;
		}
		ptr++;
		str_skipblanks(ptr);

		char* varname = str_dupvarname(ptr, false);
		ScriptVariable& dv = *(ec->registerScriptVariable(varname));
		AOEFREE(varname);

		switch(*format){
		case 'i': {
			dv.setval((int) NTOH_INT32(*((int32_t*) (currentValue))));
			currentValue += 4;
		}
			break;

		case 'I': {
			dv.setval((int) NTOH_UINT32(*((uint32_t*) (currentValue))));
			currentValue += 4;
		}
			break;

		case 'c': {
			dv.setval((int) NTOH_INT8(*((int8_t*) (currentValue))));
			currentValue += 4;
		}
			break;

		case 'C': {
			dv.setval((int) NTOH_UINT8(*((uint8_t*) (currentValue))));
			currentValue += 1;
		}
			break;

		case 'b': {
			syntaxError(ptr, "Byte array unsupported in script engine.");
		}
			break;

		case 's': {
			unsigned long size;
			if(isSized) {
				// Variable-length sizes are only valid with sized structures.
				size = strnlen(currentValue, bufferLen - (currentValue - buffer)) + 1;
			} else {
				size = 256;
			}
			dv.setval(currentValue);
			currentValue += size;
		}
			break;

		case 'S': {
			// Parse subFormat.
			// Example parameter : unpack("!S", dstStruct:"!i");
			str_skipblanks(ptr);
			if(*ptr != ':') {
				syntaxError(ptr,
				            "unpack: Sub structure format missing. The parameter needs a structure format like this : unpack(\"!S\", dstStruct:\"!i\");");
				return;
			}
			ptr++;
			ScriptVariable fmt;
			if(subParse(runCode, fmt, OP_NUL))
				return;
			const char* subFormat = fmt.getstr();

			assert(subFormat);
			assert(isSized);

			char* dstStruct;
			unsigned long size;
			if(format[1] == '\0' && *subFormat == '!') {
				size = bufferLen - (currentValue - buffer); // Deduce the size of the sub structure
				dstStruct = (char*) AOEMALLOC(size + 4);
				memcpy(dstStruct + 4, currentValue, size);
				*((uint32_t*) dstStruct) = HTON_UINT32(size + 4); // Restore the size header of the sub structure
			} else {
				size = getPackedStructureLength(subFormat, currentValue);
				dstStruct = (char*) AOEMALLOC(size);
				memcpy(dstStruct, currentValue, size);
			}

			dv.setval(subFormat, dstStruct);

			currentValue += size;
			assert(format[1] != '\0' || (currentValue == buffer + bufferLen));

		}
			break;

		default: {
			syntaxError(ptr, "Unsupported format string.");
		}
			break;
		}

	} while(*(++format));

	str_skipblanks(ptr);
	if(*ptr != ')') {
		syntaxError(ptr, "unpack: Too many paramters for this structure. ) expected.");
	}

	ptr++;
}

// This function evolves with different instruction types
// QC:B
void ScriptedFunction::setInstr(const char* newInstr) {
	/*XXX just for debug. Not really a mutex.*/static bool sillyMutex = false;
	if(newInstr != instr) {
		assert(!(newInstr && instr));
		if(instr) {
			if(strcmp(instr, "fct") == 0) {
				instr = NULL;

				assert(!sillyMutex);
				sillyMutex = true;
				delete instr_data.instr_fct.fctParams;
				assert(sillyMutex);
				sillyMutex = false;

				delete instr_data.instr_fct.fctVar;
			} else if(strcmp(instr, "dot") == 0) {
				instr = NULL;
				delete ec;
				ec = instr_data.instr_dot.currentContext;
			} else if(strcmp(instr, "txtfct") == 0) {
				instr = NULL;
				Context* &htbl = instr_data.instr_fct.htbl;
				Context* &oldsuper = instr_data.instr_fct.oldsuper;
				if(htbl) {
					assert(oldsuper);
					htbl->setsuper(oldsuper);
				}
				delete instr_data.instr_fct.fctParams;
				delete instr_data.instr_fct.fctVar;
			}
		}

		instr = newInstr;
	}
}

// Retourne vrai si string commence par l'instruction instr.
// Le pointeur string est place a la fin de l'instruction si la comparaison renvoie vrai
// Cette fonction est ultra-rapide si les premiers caracteres ne correspondent pas.
// GC:S (tres bien teste, mais encore optimisable)
bool ScriptedFunction::instrCmp(char* &ptr, const char* instr) {
	// Compare le premier caractere pour plus de vitesse en cas d'echec
	if(*ptr == *instr) {
		char* left = ptr; // Memorise le point de depart en cas d'echec

		// Compare les autres caracteres
		while(*(++ptr) == *(++instr));

		if(!*ptr) {
			// L'instruction s'arrete juste a la fin du fichier !
			return true;
		}

		// On va verifier que l'instruction au moins terminee.
		if(*instr) {
			// En realite, il s'agit d'un nom de variable/fonction qui commence comme une instruction
			ptr = left;
			return false;
		}

		// On verifie que l'instruction est suivie d'un caractere non-alphanumerique
		if((*ptr >= 'A' && *ptr <= 'Z') || (*ptr >= 'a' && *ptr <= 'z') || (*ptr >= '0' && *ptr <= '9') || *ptr == '_') {
			ptr = left;
			return false;
		}

		// On a bien une instruction
		return true;
	}

	// Le premier caractere ne correspond meme pas.
	return false;
}

// Saute des blancs en prenant en charge les commentaires
// QC:B (a montré des signes de faiblesse. La nouvelle version n'est pas testée à 100%)
char* ScriptedFunction::str_skipblanks(char* &ptr) {
	while(*ptr) {
		// Elimine les commentaires
		if(*ptr == '/') {
			ptr++;
			if(*ptr == '/') {
				// Commentaire sur une ligne
				while(*ptr != '\n' && *ptr != '\r') {
					if(!*ptr)
						return ptr;
					ptr++;
				}
			} else if(*ptr == '*') {
				// Bloc de commentaires
#ifdef SCRIPTLOG
				char* sptr = ptr;
				syslog("Skipping comment.");
#endif
				while(*ptr != '*' || ptr[1] != '/') {
					if(!*ptr)
						return ptr;
					ptr++;
				}
#ifdef SCRIPTLOG
				syslog("Comment : %d characters.", ptr-sptr);
#endif

				// Saute le */ final
				ptr += 2;
				if(!*ptr)
					return ptr;
			} else {
				return --ptr; // Ce n'est pas un commentaire
			}
		} else if(*ptr == '#') {
			if(ptr[1] == '!') {
				ptr += 2;
				// Commentaire de type #!. On elimine jusqu'a la fin de la ligne
				while(*ptr != '\n' && *ptr != '\r') {
					if(!*ptr)
						return ptr;
					ptr++;
				}
			}
		} else if(*ptr > ' ') {
			return ptr;
		}

		ptr++;
	}

	return ptr;
}

// QC:S
char* ScriptedFunction::str_skipvarname(char* &ptr) {
	while((*ptr >= 'A' && *ptr <= 'Z') || (*ptr >= 'a' && *ptr <= 'z') || (*ptr >= '0' && *ptr <= '9') || *ptr == '_') {
		ptr++;
	}

	return ptr;
}

// QC:A
char* ScriptedFunction::str_tokens(char* &string, const char* sepList, bool eliminateSpecialChars) {
	bool inQuotes = false;

	if(!*sepList) {
		// Chaine de separateurs de taille nulle
		// Aucun interet !
		return string;
	}

	// Teste chaque caractere de la chaine juqu'au caractere nul
	while(*string) {
		if(eliminateSpecialChars) {
			// Le \ saute le caractere suivant
			if(inQuotes && *string == '/') {
				string++;
				if(*string == '\0') {
					syntaxError(string, "strange thing around a /");
				}
			}

			// Gestion des quotes
			if(*string == '"') {
				inQuotes = !inQuotes;
			}

			// Vire les commentaires
			while(!inQuotes && *string == '/') {
				string++;

				if(*string == '/') {
					// Commentaire sur une ligne
					string++;
					string = str_tokens(string, "\r\n", false);
				} else if(*string == '*') {
					// Bloc de commentaires
					while(*string != '*' && *(++string) && *string != '/');
				}
			}
		}

		// Verifie que le caractere n'est pas dans la liste de separateurs.
		// La liste se termine par \0.
		const char* separateur = sepList;
		if(!inQuotes)
			while(*separateur) {
				if(*string == *separateur) {
					// Le caractere est un separateur, arret du traitement
					return string;
				}
				separateur++;
			}

		string++;
	}

	// Aucun caractere n'est un separateur, retourne la chaine vide
	return string;
}

// QC:P
void ScriptedFunction::syntaxError(char* ptr, const char* message) {
	syslog("+> Syntax error : %s", message);

	if(ptr != NULL) {
		char l_near[60];
		memcpy(l_near, ptr - 20, sizeof(l_near) - 1);
		for(unsigned int i = 0; i < sizeof(l_near); i++)
			if((i < 20 || l_near[i] > 0) && l_near[i] < 32)
				l_near[i] = ' ';
		l_near[sizeof(l_near) - 1] = '\0';
		syslog("| near \"%s\"", l_near);
		syslog("|___________________________T");
	}

	abort();
}


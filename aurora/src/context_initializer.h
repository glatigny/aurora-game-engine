#ifndef CONTEXT_INITIALIZER_H
#define CONTEXT_INITIALIZER_H

/** This class just calls registerClass with all known classes when called.
 * This way, the context and all its subcontexts can call native functions.
 *
 * This is kept separated from context to avoid too many dpendencies on context.cpp.
 */
class ContextInitializer {
public:
	static void registerAllNativeClasses(Context* c);
};

#endif /* CONTEXT_INITIALIZER_H */

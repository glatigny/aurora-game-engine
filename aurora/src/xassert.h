#ifndef XASSERT_H
#define XASSERT_H

#include <assert.h>

// xassert, xadvice, xwarn and xerror definition.

/* Rules for advice/warning/error/assertion usage :
 *
 * xadvice signals 'tips and tricks'. For example, valid values that lead to lower performances.
 * xwarn allows to signal an error that allows to continue viable execution.
 * xerror signal fatal errors such as invalid values or states that would lead to undefined behaviour.
 * xassert signal that a problem comes from wrong logic or a wrong algorithm.
 */

#if defined(WIN32) && defined(LOG_WINDOW_SUPPORT)
extern void win32ShowError(const char* message);
#define ASSERTION_FAILED(...)\
	do { char message[4092]; \
	syslog(__VA_ARGS__); \
	snprintf(message, 4090, __VA_ARGS__); \
	message[4090] = '\0'; \
	win32ShowError(message); } while(0)
#else
#define ASSERTION_FAILED syslog
#endif /* WIN32 && LOG_WINDOW */


#ifndef __GNUC__

#ifdef NDEBUG
#define xassert(...)
#else /* NDEBUG */
// DIRTY TRICK HERE. The +0 avoids the syntax error when no extra parameter is passed.
#define xassert(condition, diagnosis, ...) \
do { \
	if(!(condition)) { \
		ASSERTION_FAILED("** ASSERTION FAILED:"#condition "\n at %s:%d\n (in %s)\n\n" diagnosis, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__ +0); \
		abort(); \
	} \
} while(0)
#endif /* NDEBUG */

#ifdef NADVICE
#define xadvice(...)
#else /* NADVICE */
#define xadvice(condition, advice, ...) \
do { \
	if(!(condition)) { \
		syslog("ADVICE: " advice, __VA_ARGS__ +0); \
	} \
} while(0)
#endif /* NADVICE */

#ifdef NWARNING
#define xwarn(...)
#else /* NWARNING */
#define xwarn(condition, warning, ...) \
do { \
	if(!(condition)) { \
		syslog("WARNING: " warning, __VA_ARGS__ +0); \
	} \
} while(0)
#endif /* NWARNING */

#ifdef NERROR
#define xerror(...)
#else /* NERROR */
#define xerror(condition, error, ...) \
do { \
	if(!(condition)) { \
		ASSERTION_FAILED("FATAL ERROR: " error, __VA_ARGS__ +0); \
		abort(); \
	} \
} while(0)
#endif /* NERROR */

#else /* __GNUC__ */

#ifdef NDEBUG
#define xassert(...)
#else /* NDEBUG */
#define xassert(condition, diagnosis, ...) \
do { \
	if(!(condition)) { \
		ASSERTION_FAILED("** ASSERTION FAILED:"#condition "\n at %s:%d\n (in %s)\n\n" diagnosis, __FILE__, __LINE__, __func__, ## __VA_ARGS__); \
		abort(); \
	} \
} while(0)
#endif /* NDEBUG */

#ifdef NADVICE
#define xadvice(...)
#else /* NADVICE */
#define xadvice(condition, advice...) \
do { \
	if(!(condition)) { \
		syslog("ADVICE: " advice); \
	} \
} while(0)
#endif /* NADVICE */

#ifdef NWARNING
#define xwarn(...)
#else /* NWARNING */
#define xwarn(condition, warning...) \
do { \
	if(!(condition)) { \
		syslog("WARNING: " warning); \
	} \
} while(0)
#endif /* NWARNING */

#ifdef NERROR
#define xerror(...)
#else /* NERROR */
#define xerror(condition, error...) \
do { \
	if(!(condition)) { \
		ASSERTION_FAILED("FATAL ERROR: " error); \
		abort(); \
	} \
} while(0)
#endif /* NERROR */

#endif /* __GNUC__ */

#endif /* XASSERT_H */

/* Pointer manipulation macros.
 * These macros help to manipulate strings, etc ...
 */

#ifndef POINTER_UTILS_H
#define POINTER_UTILS_H

#ifdef WIN32
#include <malloc.h>
#else
#include <alloca.h>
#endif

#define NULL_AND_DELETE(obj, type) \
	do { if(obj) { type _##obj##_tmp = obj; obj = NULL; delete _##obj##_tmp; } } while(0)

/* Duplicates a string to a stack buffer. The resulting buffer is just big enough to hold the new string.
 * NULL pointers are passed on, so STACK_STRDUP(NULL, 1024) == NULL
 * Usage :
 * char* srcString = "hello world";
 * const char* destString;
 * STACK_STRDUP(destString, srcString, 8192);
 * // destString points to a temporary buffer on the stack. const is optional and const-ness is provided for ease of use.
 *
 * This is NOT a function : it expands to many statements, so use it only to set a variable.
 */
// QC:P
#define STACK_STRDUP(dst, src, maxlen) \
	if(src) { \
		size_t srclen = strnlen((src), (maxlen)-1)+1; \
		(dst)=(const char*)alloca(srclen); \
		memcpy((char*)(dst), (src), srclen); \
	} else { (dst) = NULL; } \
	do;while(0)

// Duplicates a semicolumn-terminated parameter onto the stack
// QC:G (may have unknown behaviour in specific cases)
#define STACK_PARAMDUP(dst, src, maxlen) { \
	if(src) { \
		const char* end = strchrnul((src), ';'); \
		size_t srclen; \
		srclen = MIN(end-(src), (maxlen)); \
		dst=(char*)alloca(srclen); \
		memcpy((char*)dst, (src), srclen); \
		dst[srclen] = '\0'; \
	} else { dst = NULL; } \
} do;while(0)

#endif /* POINTER_UTILS_H */

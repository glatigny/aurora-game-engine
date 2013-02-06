#ifndef GLOBALS_H
#define GLOBALS_H

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifdef MEMTRACKING
#define AOEMALLOC aoeMalloc
#define AOEFREE aoeFree
#define AOESTRDUP aoeStrdup
extern size_t aoeMallocTotal;
extern size_t aoeMallocPeak;
extern size_t aoeMallocCount;
extern void* aoeMalloc(size_t size);
extern void aoeFree(void* ptr);
extern char* aoeStrdup(const char* str);
#define AOEVIDALLOC aoeVidAlloc
#define AOEVIDFREE aoeVidFree
extern size_t aoeVidAllocTotal;
extern size_t aoeVidAllocPeak;
extern size_t aoeVidAllocCount;
extern void aoeVidAlloc(void* ptr, size_t size);
extern void aoeVidFree(void* ptr);
#else
#define AOEMALLOC malloc
#define AOEFREE free
#define AOESTRDUP strdup
#define AOEVIDALLOC(...) do {} while(0)
#define AOEVIDFREE(...) do {} while(0)
#endif

#include "xassert.h"
#include "pspec/utils.h"
#include "macros/pointer_utils.h"
#include "macros/byte_order.h"
#include "blocking.h"
#include "deprecation.h"

#ifdef POINTER_TRACKING
#include <map>
#include <set>
class ScriptableObject;
typedef std::map<ScriptableObject**, ScriptableObject*> PointerMap;
extern PointerMap pointerMap;
extern void trackPointer(ScriptableObject* &pointer);
extern void untrackPointer(ScriptableObject* &pointer);
extern void trackPointer(char* &pointer);
extern void untrackPointer(char* &pointer);
extern void replacePointer(ScriptableObject* oldValue, ScriptableObject* newValue);
#else
#define trackPointer(...) do {} while(0)
#define untrackPointer(...) do {} while(0)
#define replacePointer(...) do {} while(0)
#endif

#ifdef BE
#define CONST_NAME_TO_ID(n) ((const uint32_t) JOIN(JOIN(QUOTE,n),QUOTE))
#else
#define CONST_NAME_TO_ID(n) ( \
	(((const uint32_t) (n) & 0xFF) << 24) | \
	(((const uint32_t) (n) & 0xFF00) << 8) | \
	(((const uint32_t) (n) & 0xFF0000) >> 8) | \
	(((const uint32_t) (n) & 0xFF000000) >> 24) )
#endif

#define NAME_TO_ID(nameString) (*((const uint32_t*)(nameString)))
#define ID_TO_NAME(id) ((const char*)(&id))

#define MIN(a, b) ((a)<(b) ? (a) : (b))
#define MAX(a, b) ((a)>(b) ? (a) : (b))
#define CLIP(a,b,c) (MIN(MAX((a),(b)),(c))) // Clips a between b and c

#ifdef FLOAT_COORD
#define atoc atof
#include <math.h>
#ifdef DOUBLE_COORD
#define absc fabs
#else // DOUBLE_COORD
#define absc fabsf
#endif // DOUBLE_COORD
#else // FLOAT_COORD
#define atoc atoi
#define absc abs
#endif // FLOAT_COORD
#define distc(a,b) absc((a)-(b))

#ifdef FLOAT_DURATION
#define atod atof
#include <math.h>
#ifdef DOUBLE_DURATION
#define absd fabs
#else
#define absd fabsf
#endif
#else
#define atod atoi
#define absd abs
#endif

/*--- VARIABLES ---*/
class AlwaysBlocking : public Blocking {
	bool isFinished() { return false; }
};
extern AlwaysBlocking alwaysBlocking;

class NeverBlocking : public Blocking {
	bool isFinished() { return true; }
};
extern NeverBlocking neverBlocking;

/*--- DEFINITION ---*/
int nextPowerofTwo(int x);
const char* relPath(const char* baseFile, const char* relFile);
void suppressDotPaths(char* filename);
unsigned long parseBufferSize(const char* &format);
char* packStructure(const char* format, ...);
void unpackStructure(const char* format, const char* buffer, ...);
char* packedStructureDup(const char* format, const char* buffer);
unsigned long getPackedStructureLength(const char* format, const char* buffer);

// QC:P
inline unsigned int getPackedStructureLength(const char* buffer) {
	return NTOH_INT32(*((uint32_t*) (buffer)));
}

unsigned int getPackedStructureVariableCount(const char* format);
unsigned long computePackedStructureLength(const char* format);
bool stringToInt(const char* string, int* i);
bool stringToFlags(const char* string, uint32_t* i);
bool stringToBool(const char* string, bool* b);
int stringToIntList(const char* string, int* v0, int* v1, int* v2, int* v3);
bool stringToCoord(const char* string, coord* i);
int stringToCoordList(const char* string, coord* v0, coord* v1, coord* v2, coord* v3);
int codepointToUTF8(char* outBuf, unsigned long codepoint);
int UTF8ToCodepoint(char* t, unsigned long& codepoint);
int getUTF8CharLen(unsigned long codepoint);
coord modc(coord c, coord m); // Get modulo of a coordinate
dur modd(dur d, dur m); // Get modulo of a duration


#endif /* GLOBALS_H */

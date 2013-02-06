#ifndef AURORA_CONSTS
#define AURORA_CONSTS


// Link WIN32 and _WIN32
#ifdef WIN32
#ifndef _WIN32
#define _WIN32
#endif
#endif
#ifdef _WIN32
#ifndef WIN32
#define WIN32
#endif
#endif

// Set UNIX
#if defined(unix) || defined(__unix__) || defined(__unix)
#define UNIX
#endif

#ifdef GPU_IMAGEPROCESS
#define OPENGL_ENABLE_FBO
#define TEXTURE_SLOTS 2
#else
#define TEXTURE_SLOTS 1
#endif

#ifndef FORMAT_CHECK_BUFFER_LENGTH
#define FORMAT_CHECK_BUFFER_LENGTH 256
#endif

// Number of background task queues
#ifndef MAX_BG_QUEUES
#define MAX_BG_QUEUES 8
#endif

// DOUBLE_COORD implies FLOAT_COORD
#ifdef DOUBLE_COORD
#ifndef FLOAT_COORD
#define FLOAT_COORD
#endif
#endif
// Defines the coord type
#ifdef FLOAT_COORD
#ifdef DOUBLE_COORD
typedef double coord;
#else /* DOUBLE_COORD */
typedef float coord;
#endif /* DOUBLE_COORD */
typedef coord floatcoord;
#else /* FLOAT_COORD */
typedef int coord;
typedef float floatcoord;
#endif /* FLOAT_COORD */

// DOUBLE_DURATION implies FLOAT_DURATION
#ifdef DOUBLE_DURATION
#ifndef FLOAT_DURATION
#define FLOAT_DURATION
#endif
#endif
// Defines the dur type
#ifdef FLOAT_DURATION
#ifdef DOUBLE_DURATION
typedef double dur;
#else /* DOUBLE_DURATION */
typedef float dur;
#endif /* DOUBLE_DURATION */
#else /* FLOAT_DURATION */
typedef int dur;
#endif /* FLOAT_DURATION */

// SCRIPT_DEBUGGER needs SYSLOG and DEBUG_KEYS
#ifdef SCRIPT_DEBUGGER
#ifndef SYSLOG
#define SYSLOG
#endif
#ifndef DEBUG_KEYS
#define DEBUG_KEYS
#endif
#endif /* SCRIPT_DEBUGGER */

#ifndef AURORA_WINDOW_TITLE
#define AURORA_WINDOW_TITLE	"Aurora Studio Application"
#endif


/*-EOF-*/
#endif /* AURORA_CONSTS */

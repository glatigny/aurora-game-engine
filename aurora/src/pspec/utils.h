#ifndef PSPEC__UTILS_H
#define PSPEC__UTILS_H
/*--------- Plateform-specific utils header ----------*/

// Log and errors

#ifdef SYSLOG
#include <stdio.h>

#ifdef WIN32

extern FILE* syslogfile;
#ifdef LOG_WINDOW_SUPPORT
extern void win32Syslog(char* text);
#else
#define win32Syslog(...)
#endif
#define syslog(...) do{ fprintf(syslogfile, __VA_ARGS__); fprintf(syslogfile, "\r\n"); fflush(syslogfile); char lbuffer[4096]; sprintf(lbuffer, __VA_ARGS__); win32Syslog(lbuffer); }while(0)

#else


#define syslog(...) do{ fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); }while(0)

#endif

#else /* SYSLOG */

#define syslog(...)

#endif /* SYSLOG */


/** Returns true if filename is a directory.
 * @param filename the name of the file to test.
 * @return true if the file is a directory, false otherwise.
 */
bool isDirectory(const char* filename);

/** Checks that byte ordering is coherent.
 * @return true if the compile-time byte ordering corresponds to the hardware.
 */
bool checkByteOrder();

#ifdef SDL_SUPPORT
#include <SDL/SDL.h>
#include <SDL/SDL_timer.h>
#endif /* SDL_SUPPORT */

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#define WINVER 0x0500
#define	_WIN32_WINNT 0x0500
extern "C" {
#include <winsock2.h>
#include <windows.h>
}
#include <stdlib.h>
#include <mmsystem.h>

#define getTime (unsigned long)timeGetTime
//#define waitMillis Sleep
#define bzero ZeroMemory
#define snprintf _snprintf

#include "win32/time.h"

char* strchrnul(const char* s, int c);

#define fseeko fseek
#define ftello ftell

#include "win32/dirent.h"

// Define for math.h ( M_PI, M_PI_2 )
#define _USE_MATH_DEFINES

// Creation of a warning list for Microsoft Visual Studio "wonderful" compiler
#pragma warning(push)
#pragma warning(disable:4100)
#pragma warning(disable:4127)
#pragma warning(disable:4131)
#pragma warning(disable:4244)
#pragma warning(disable:4250)
#pragma warning(disable:4267)
#pragma warning(disable:4706)
#pragma warning(disable:4996)

// Cast
#pragma warning(disable:4311)
#pragma warning(disable:4312)

#else /*WIN32*/

#include <arpa/inet.h>
#include <dirent.h>

#ifdef SDL_SUPPORT

#define getTime (unsigned long)SDL_GetTicks
#define waitMillis SDL_Delay

#else /* SDL_SUPPORT */

#include <unistd.h>
#define getTime() (unsigned long)0 /* FIXME : getTime unimplemented if no SDL under linux. */
#define waitMillis(time) usleep((time)*1000)

#endif /* SDL_SUPPORT */

#endif /* WIN32 */


#define HTON_INT8(H) ((int8_t)(H))
#define HTON_UINT8(H) ((uint8_t)(H))
#define HTON_INT32(H) ((int32_t)htonl(H))
#define HTON_UINT32(H) (htonl(H))
#define NTOH_INT8(H) ((int8_t)(H))
#define NTOH_UINT8(H) ((uint8_t)(H))
#define NTOH_INT32(H) ((int32_t)ntohl(H))
#define NTOH_UINT32(H) (ntohl(H))

#define randomDouble rand
#endif


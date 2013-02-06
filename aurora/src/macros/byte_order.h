#ifndef BYTE_ORDER_H
#define BYTE_ORDER_H

// This file will set either LE or BE

#if ( ! defined(LITTLE_ENDIAN) && ! defined(BIG_ENDIAN) ) || ( defined(LITTLE_ENDIAN) && defined(BIG_ENDIAN) )

#ifdef WIN32
#define LE // Ehh, seen big endian WIN32 machines ? !!!
#else

#if defined(__hppa__) || \
 defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
 (defined(__MIPS__) && defined(__MISPEB__)) || \
 defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC) || \
 defined(__sparc__)
#define BE
#else
#define LE
#endif

#endif

#else

// Either LITTLE_ENDIAN or BIG_ENDIAN is set.
#ifdef BIG_ENDIAN
#define BE
#else
#define LE
#endif

#endif

#endif /* BYTE_ORDER_H */


#ifndef _BRCTL_H
#define _BRCTL_H

#ifdef __cplusplus
extern "C" {
#endif

#define ALIGN1 __attribute__((aligned(1)))
#define ALIGN2 __attribute__((aligned(2)))
//#define USE_FEATURE_BRCTL_FANCY(...) (...)
//#define USE_FEATURE_BRCTL_SHOW(...) (...)

#define ENABLE_FEATURE_BRCTL_FANCY 1
#define ENABLE_FEATURE_BRCTL_SHOW 1

/* Size-saving "small" ints (arch-dependent) */
#if defined(i386) || defined(__x86_64__) || defined(__mips__) || defined(__cris__)
/* add other arches which benefit from this... */
typedef signed char smallint;
typedef unsigned char smalluint;
#else
/* for arches where byte accesses generate larger code: */
typedef int smallint;
typedef unsigned smalluint;
#endif

//typedef uint8_t u8;

//typedef signed char uint8_t;
/* 2010-1223 Mook */
typedef unsigned char uint8_t;

#define EXIT_SUCCESS 0

#ifdef __cplusplus
}
#endif

#endif

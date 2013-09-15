#ifndef types_h
#define types_h

//#include <inttypes.h>

#define talloc(t) malloc(sizeof( t ))

#define nano  1
#define micro nano  * 1000
#define milli micro * 1000
#define sec   milli * 1000
#define min   sec * 60
#define hour  min * 60
#define day   hour * 24

/** 
 * We express the timestamps as nanos
 * relative to something.
 *
 * Relative to the 1.1.1970 this will surfice
 * till the year 2569.
 *
 * TODO: We might should change this to __uint128_t
 *       This would be much longer than the universe exists,
 *       but much shorter till the heat death of the universe.
 *       Support till the end of the universe would require __uint512_t
 */ 
typedef uint64_t tstamp;

typedef char* Str;

#endif
